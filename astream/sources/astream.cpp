// astream.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <vector>
#include <string>

#include "HttpClient.h"
#include "json_helpers.h"
#include "astream_ErrorCode.h"

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
namespace po = boost::program_options;

using namespace std;

const string DefaultJobFormat("%|*id*| %|5t|%|*OriginalDocName*| %|40t|%|*TemplateName*| %|60t|%|*PreparingStatus*|");

// sub commands
int process_job			( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);
int process_job_add		( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);
int process_job_list		( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);
int process_job_print	( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);
int process_job_remove	( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);
int process_job_show		( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm);

bool ValidatePrintRange(const std::string& input, std::string& output);

/*
* Print EPreparingStatus names
*/
std::ostream& operator<<(std::ostream& os, const EPreparingStatus& p)
{
	static string names[] = {
		"Uploading",
		"Waiting",
		"WaitingForPDFToAFPConverting",
		"PDFToAFPConverting",
		"PDFToAFPConverted",
		"PDFToAFPConvertionError",
		"WaitingForIndexing",
		"Indexing",
		"Indexed",
		"IndexingError",
		"UploadingError",
		"Analyzing",
		"AnalyzingError",
		"WaitingForParams",
		"WaitingForPSToPDFConverting",
		"PSToPDFConverting",
		"PSToPDFConverted",
		"PSToPDFConvertionError",
	};
	os << names[p];
	return os;
};

/*
 * Print usage information
*/
void print_usage(po::options_description& options)
{
	std::cout << "Usage: astream <command> [<args>]" << endl;
	//std::cout << "       astream [ -h | --help | -v | --version ]" << endl;
	std::cout << "       astream [ -h | --help ]" << endl;
	std::cout << options << endl;
	std::cout << "Commands:" << endl;
	std::cout << "   job         Manage jobs" << endl;
	std::cout << "   printqueue  Manage print queue" << endl << endl;
	std::cout << "Run 'astream <command> --help' for more information on a command." << endl;
}

int main(int argc, const char* const argv[])
{
	// Generic options
	po::options_description generic("Generic options");
	generic.add_options()
		//("version,v", "print version")
		("help,h", "produce help message")
		;

	// Hidden options
	po::options_description hidden;
	hidden.add_options()
		("command", po::value< string >(), "command")
		("subargs", po::value< std::vector<std::string> >(), "arguments")
		;
	po::positional_options_description pos;
	pos.add("command", 1)
		.add("subargs", -1);

	// Declare a group of options that will be visible for help
	po::options_description visible;
	visible.add(generic);

	// Declare all options that will be parsed
	po::options_description cmdline_options;
	cmdline_options.add(generic).add(hidden);

	po::variables_map vm;
	try
	{
		// Parse command line
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(cmdline_options).positional(pos).allow_unregistered().run();
		po::store(parsed, vm);
		po::notify(vm);

		if (vm.count("command")==0)
		{
			//if (vm.count("help"))
			{
				print_usage(visible);
				return ERROR_ASTREAM_SUCCESS;
			}
		}
		else
		{
			// Server options
			po::options_description server("Server options");
			server.add_options()
				("server,s", po::value<string>()->default_value("127.0.0.1"), "astream server address")
				("port,p", po::value<int>()->default_value(8100), "astream server port")
				;

			std::string cmd = vm["command"].as<std::string>();
			if (cmd == "job")
			{
				return process_job(parsed, server, vm);
			}
			else if (cmd == "printqueue")
			{
				std::cout << "'" << cmd << "' command is not yet implemented." << endl;
				return 1;
				//return process_printqueue( parsed, server, vm);
			}
			else
			{
				std::cout << "'" << cmd << "' is not an astream command." << endl;
				std::cout << "See 'astream --help'." << endl;
				return ERROR_ASTREAM_UNKNOWN_COMMAND;
			}
		}
	}

	// options obligatoires
	catch (boost::program_options::required_option e)
	{
		if (vm.count("help"))
		{
			print_usage(visible);
			return ERROR_ASTREAM_SUCCESS;
		}
		else
		{
			cout << e.what() << endl;
			std::cout << "See 'astream --help'." << endl;
			return ERROR_ASTREAM_REQUIRED_OPTION_MISSING;
		}
	}
	// options inconnues
	catch (boost::program_options::unknown_option e)
	{
		cout << e.what() << endl;
		std::cout << "See 'astream --help'." << endl;
		return ERROR_ASTREAM_UNKNOWN_OPTION;
	}
	// autres erreurs
	catch (boost::program_options::error e)
	{
		cout << e.what() << endl;
		std::cout << "See 'astream --help'." << endl;
		return ERROR_ASTREAM_FAILED;
	}

	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* job command
*
**********************************************************************/
int process_job(po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	opts.erase(opts.begin());

	// Hidden options
	po::options_description hidden;
	hidden.add_options()
		("jobcommand", po::value< string >(), "job command")
		("jobsubargs", po::value< std::vector<std::string> >(), "job subargs")
		;
	po::positional_options_description pos;
	pos.add("jobcommand", 1)
		.add("jobsubargs", -1);

	po::options_description cmd_desc;
	cmd_desc.add(common_options);

	// Parse again...
	po::options_description cmd;
	cmd.add(common_options).add(hidden);

	po::parsed_options parsed2 = po::command_line_parser(opts).options(cmd).positional(pos).allow_unregistered().run();
	po::store(parsed2, vm);
	po::notify(vm);

	if (vm.count("help") && !vm.count("jobcommand"))
	{
		std::cout << "Usage: astream job [<command>] [<options>] [<name>]" << endl << endl;
		std::cout << "Manage jobs" << endl << endl;
		std::cout << "Commands:" << endl;
		std::cout << "   With no arguments, shows a list of existing jobs.Several subcommands are available to perform operations on the jobs." << endl << endl;
		std::cout << "   add         Add a job" << endl;
		std::cout << "   list        shows a list of existing jobs" << endl;
		std::cout << "   print       Send the job <name> to the print queue" << endl;
		std::cout << "   remove      Remove the job <name>" << endl;
		std::cout << "   show        Gives some information about the job <name>" << endl << endl;
		std::cout << "Run 'astream job <command> --help' for more information on a job command." << endl;
		std::cout << cmd_desc << endl;
		return ERROR_ASTREAM_SUCCESS;
	}
	else if (vm.count("jobcommand"))
	{
		std::string cmd = vm["jobcommand"].as<std::string>();
		if (cmd == "add")
		{
			return process_job_add(parsed2, common_options, vm);
		}
		else if (cmd == "list")
		{
			return process_job_list(parsed2, common_options, vm);
		}
		else if (cmd == "print")
		{
			return process_job_print(parsed2, common_options, vm);
		}
		else if (cmd == "remove")
		{
			return process_job_remove(parsed2, common_options, vm);
		}
		else if (cmd == "show")
		{
			return process_job_show(parsed2, common_options, vm);
		}
		else
		{
			std::cout << "'" << cmd << "' is not an astream job command." << endl;
			std::cout << "See 'astream job --help'." << endl;
			return ERROR_ASTREAM_UNKNOWN_COMMAND;
		}
	}
	else // list
	{
		return process_job_list(parsed2, common_options, vm);
	}

	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* Ajout d'un Job
*
**********************************************************************/
int process_job_add( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// addjob command has the following options:
	po::options_description addjob_desc("addjob options");
	addjob_desc.add_options()
		("friendlyName,n", po::value<string>(), "job's mame")
		("templateName,u", po::value<string>(), "template mame used for pdf job")
		("templateFile,t", po::value<string>(), "template file path used for pdf job")
		("formdef,f", po::value<string>(), "formdef used for afp job")
		("range,r", po::value<string>(), "page range to print - formatted as follow : [min-page;max-page]")
		;
	addjob_desc.add(common_options);

	po::options_description hidden;
	hidden.add_options()
		//("input-file", po::value< vector<string> >()->composing(), "input file")
		("input-file", po::value< string >(), "input file") // 1 seul job � la fois pour le moment
		;
	po::positional_options_description pos;
	pos.add("input-file", -1);

	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	opts.erase(opts.begin());

	// Parse again...
	po::options_description cmd;
	cmd.add(hidden).add(addjob_desc);
	po::store(po::command_line_parser(opts).options(cmd).positional(pos).run(), vm);

	if (boost::filesystem::exists("astream.conf"))
	{
		// Declare a group of options that will be allowed only in config file
		po::options_description config_file_options;
		config_file_options.add(common_options);

		// parse config file
		po::store(po::parse_config_file<char>("astream.conf", config_file_options), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: astream job add [<options>] <filename>" << endl << endl;
		std::cout << "Add a job to alphastream" << endl << endl;
		std::cout << addjob_desc << endl;
		return ERROR_ASTREAM_SUCCESS;
	}

	// si pas de job fourni, on ne va pas plus loin
	if (!vm.count("input-file"))
	{
		return ERROR_ASTREAM_NO_INPUT_FILE_PROVIDED;
	}

	CHttpClient l_HttpClient;

	// upload du job
	vector<S_JobInfos> l_VectJobInfos;
	try
	{
		cout << "uploading job to " << vm["server"].as<string>().c_str() << ":" << vm["port"].as<int>() << " ... " << flush;

		l_VectJobInfos = l_HttpClient.UploadJob(
			vm["server"].as<string>().c_str(),
			vm["port"].as<int>(),
			vm["input-file"].as<string>().c_str(),
			(vm.count("friendlyName") == 0) ? NULL : vm["friendlyName"].as<string>().c_str(),
			(vm.count("templateFile") == 0) ? NULL : vm["templateFile"].as<string>().c_str(),
			(vm.count("templateName") == 0) ? NULL : vm["templateName"].as<string>().c_str(),
			(vm.count("formdef") == 0) ? NULL : vm["formdef"].as<string>().c_str());

		// si le fichier n'a pas pu �tre upload�
		if (l_VectJobInfos.size() == 0)
		{
			cout << " => FAILED" << endl;
			return ERROR_ASTREAM_JOB_UPLOAD_FAILED;
		}
		cout << " => SUCCESS" << endl;
	}
	catch (CHttpClientException& e)
	{
		cout << " => FAILED (" << e.what() << ")" << endl;
		return ERROR_ASTREAM_JOB_UPLOAD_FAILED;
	}

	// print du job
	if (vm.count("range"))
	{
		string l_str_PrintRangeFormatted;
		if (!ValidatePrintRange(vm["range"].as<string>(), l_str_PrintRangeFormatted))
		{
			cout << "bad format value for --range option";
			return ERROR_ASTREAM_PRINTRANGE_BAD_FORMAT;
		}

		bool l_b_Loop = true;
		int l_i_IdJob = l_VectJobInfos.front().m_i_JobId;

		// on attente que le job soit converti
		S_JobInfos l_JobInfos;
		do
		{
			// on r�cup les infos du job
			try
			{
				l_JobInfos = l_HttpClient.GetJobInfos(
					vm["server"].as<string>().c_str(),
					vm["port"].as<int>(),
					l_i_IdJob);
			}
			catch (CHttpClientException& e)
			{
				cout << "Waiting for job status FAILED";
				cout << " (" << e.what() << ")" << endl;
				return ERROR_ASTREAM_GET_JOB_FAILED;
			}

			// selon son status
			switch (l_JobInfos.m_EPreparingStatus)
			{
			case WaitingForParams:
			case IndexingError:
			case AnalyzingError:
			case UploadingError:
			case PDFToAFPConvertionError:
				//case PSToPDFConvertionError:
				// si dans ces �tats, ne peut faire l'impression du job
				l_b_Loop = false;
				cout << "Error : unable to add job to print queue. Job is in error state (" << l_JobInfos.m_EPreparingStatus << ")" << endl;
				break;
			case Indexed:
				// index�, on peut demander l'impression
				l_b_Loop = false;
				break;
			default:
				// pas encore pr�t
				boost::this_thread::sleep(boost::posix_time::millisec(500));
				break;
			}
		} while (l_b_Loop);

		// si job index�, on demande l'impression
		if (l_JobInfos.m_EPreparingStatus == Indexed)
		{
			try
			{
				cout << "adding job (" << l_i_IdJob << ") to print queue ... " << flush;

				bool l_b_IsJobAddedToPrintQ = l_HttpClient.PrintJob(
					vm["server"].as<string>().c_str(),
					vm["port"].as<int>(),
					l_i_IdJob,
					l_str_PrintRangeFormatted.c_str());

				cout << ((l_b_IsJobAddedToPrintQ == true) ? " => SUCCESS" : " => FAILED") << endl;
			}
			catch (CHttpClientException& e)
			{
				cout << " => FAILED (" << e.what() << ")" << endl;
				return ERROR_ASTREAM_ADD_JOB_TO_PRINTQUEUE_FAILED;
			}
		}
	}
	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* Suppression d'un Job
*
**********************************************************************/
int process_job_remove(po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	opts.erase(opts.begin());

	po::options_description hidden;
	hidden.add_options()
		("id", po::value<int>(), "job id")
		;

	// job show command has the following options:
	po::options_description visible;
	visible.add(common_options);

	po::positional_options_description pos;
	pos.add("id", -1);

	// Parse again...
	po::options_description cmd;
	cmd.add(hidden).add(visible);
	po::store(po::command_line_parser(opts).options(cmd).positional(pos).run(), vm);

	if (boost::filesystem::exists("astream.conf"))
	{
		// Declare a group of options that will be allowed only in config file
		po::options_description config_file_options;
		config_file_options.add(common_options);

		// parse config file
		po::store(po::parse_config_file<char>("astream.conf", config_file_options), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: astream job remove [<options>] <id>" << endl << endl;
		std::cout << "Remove the job <id>" << endl << endl;
		std::cout << visible << endl;
		return ERROR_ASTREAM_SUCCESS;
	}

	// si pas de id job fourni, on ne va pas plus loin
	if (!vm.count("id"))
	{
		return ERROR_ASTREAM_NO_JOBID_PROVIDED;
	}

	CHttpClient l_HttpClient;
	try
	{
		if (!l_HttpClient.RemoveJob(
			vm["server"].as<string>().c_str(),
			vm["port"].as<int>(),
			vm["id"].as<int>()))
			return 1;
	}
	catch (CHttpClientException ex)
	{
		cout << ex.what() << endl;
		return 1;
	}

	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* R�cup�ration d'un Job
*
**********************************************************************/
int process_job_show( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	opts.erase(opts.begin());

	po::options_description hidden;
	hidden.add_options()
		("id", po::value<int>(), "job id")
		;
	
	// job show command has the following options:
	po::options_description visible("show options");
	visible.add_options()
		("no-header", "Do not print column headers")
		("format,f", po::value< string >()->default_value(DefaultJobFormat), "Pretty-print job using a boost::format template.\n"
																									"Valid placeholders:\n"
																									"  *id*               - \tJob identifier\n"
																									"  *UserFriendlyName* - \tDocument name\n"
																									"  *OriginalDocName*  - \tDocument name\n"
																									"  *TemplateName*     - \tTemplate name\n"
																									"  *FormDef*          - \tAFP Formdef\n"
																									"  *PreparingStatus*  - \tPreparing status")
		;
	visible.add(common_options);

	po::positional_options_description pos;
	pos.add("id", -1);

	// Parse again...
	po::options_description cmd;
	cmd.add(hidden).add(visible);
	po::store(po::command_line_parser(opts).options(cmd).positional(pos).run(), vm);

	if (boost::filesystem::exists("astream.conf"))
	{
		// Declare a group of options that will be allowed only in config file
		po::options_description config_file_options;
		config_file_options.add(common_options);

		// parse config file
		po::store(po::parse_config_file<char>("astream.conf", config_file_options), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: astream job show [<options>] <id>" << endl << endl;
		std::cout << "Gives some information about the job <id>" << endl << endl;
		std::cout << visible << endl;
		return ERROR_ASTREAM_SUCCESS;
	}

	// si pas de id job fourni, on ne va pas plus loin
	if (!vm.count("id"))
	{
		return ERROR_ASTREAM_NO_JOBID_PROVIDED;
	}

	CHttpClient l_HttpClient;
	mValue jsonValue;
	try
	{
		l_HttpClient.GetJobInfos(
			vm["server"].as<string>().c_str(),
			vm["port"].as<int>(),
			vm["id"].as<int>(),
			jsonValue);
	}
	catch (CHttpClientException ex)
	{
		cout << ex.what() << endl;
		return 1;
	}

	try
	{
		JSON_PrettyPrinter(cout, vm["format"].as<string>(), jsonValue, vm.count("no-header") == 0);
	}
	catch (std::exception e)
	{
		cout << "Formatting exception" << endl;
		cout << e.what() << endl;
		return ERROR_ASTREAM_FAILED;
	}

	//cout << boost::format("%1% %|5t|%2% %|40t|%3% %|60t|%4%\n") % "ID" % "NAME" % "TEMPLATE" % "PREPARING STATUS";
	//cout << boost::format("%1% %|5t|%2% %|40t|%3% %|60t|%4%\n") % l_JobInfos.m_i_JobId % l_JobInfos.m_str_UserFriendlyName % l_JobInfos.m_str_TemplateName.get_value_or("---") % l_JobInfos.m_EPreparingStatus;

	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* R�cup�ration des Jobs
*
**********************************************************************/
int process_job_list(po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	for (auto it = opts.begin(); it != opts.end(); it++)
	{
		if (it->compare("list") == 0)
		{
			opts.erase(it);
			break;
		}
	}

	// job show command has the following options:
	po::options_description visible("list options");
	visible.add_options()
		("no-header", "Do not print column headers")
		("format,f", po::value< string >()->default_value(DefaultJobFormat), "Pretty-print jobs using a boost::format template.\n"
																									"Valid placeholders:\n"
																									"  *id*               - \tJob identifier\n"
																									"  *UserFriendlyName* - \tDocument name\n"
																									"  *OriginalDocName*  - \tDocument name\n"
																									"  *TemplateName*     - \tTemplate name\n"
																									"  *FormDef*          - \tAFP Formdef\n"
																									"  *PreparingStatus*  - \tPreparing status")
		;
	visible.add(common_options);

	// Parse again...
	po::options_description cmd;
	cmd.add(visible);
	po::store(po::command_line_parser(opts).options(cmd).run(), vm);

	if (boost::filesystem::exists("astream.conf"))
	{
		// Declare a group of options that will be allowed only in config file
		po::options_description config_file_options;
		config_file_options.add(common_options);

		// parse config file
		po::store(po::parse_config_file<char>("astream.conf", config_file_options), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: astream jobs [<options>]" << endl << endl;
		std::cout << "list jobs" << endl << endl;
		std::cout << cmd << endl;
		return ERROR_ASTREAM_SUCCESS;
	}

	CHttpClient l_HttpClient;
	mValue jsonValue;
	try
	{
		l_HttpClient.GetJobs(
			vm["server"].as<string>().c_str(),
			vm["port"].as<int>(),
			jsonValue);
	}
	catch (CHttpClientException ex)
	{
		cout << ex.what() << endl;
		return 1;
	}

	try
	{
		JSON_PrettyPrinter(cout, vm["format"].as<string>(), jsonValue, vm.count("no-header") == 0);
	}
	catch (std::exception e)
	{
		cout << "Formatting exception" << endl;
		cout << e.what() << endl;
		return ERROR_ASTREAM_FAILED;
	}

	//cout << boost::format("%1% %|5t|%2% %|40t|%3% %|60t|%4%\n") % "ID" % "NAME" % "TEMPLATE" % "PREPARING STATUS";
	//for (auto it = l_JobInfos.begin(); it != l_JobInfos.end(); it++)
	//{
	//	cout << boost::format("%1% %|5t|%2% %|40t|%3% %|60t|%4%\n") % it->m_i_JobId % it->m_str_UserFriendlyName % it->m_str_TemplateName.get_value_or("---") % it->m_EPreparingStatus;
	//}
	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* Impression d'un Job
*
**********************************************************************/
int process_job_print( po::parsed_options& parsed, po::options_description& common_options, po::variables_map& vm)
{
	// Collect all the unrecognized options from the first pass. This will include the
	// (positional) command name, so we need to erase that.
	std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
	opts.erase(opts.begin());

	po::options_description hidden;
	hidden.add_options()
		("id", po::value<int>(), "job id")
		;

	// job print command has the following options:
	po::options_description printjob_desc("print options");
	printjob_desc.add_options()
		("range,r", po::value<string>(), "page range to print - formatted as follow : [min-page;max-page]")
		;
	printjob_desc.add(common_options);

	po::positional_options_description pos;
	pos.add("id", -1);

	// Parse again...
	po::options_description cmd;
	cmd.add(hidden).add(printjob_desc);
	po::store(po::command_line_parser(opts).options(cmd).positional(pos).run(), vm);

	if (boost::filesystem::exists("astream.conf"))
	{
		// Declare a group of options that will be allowed only in config file
		po::options_description config_file_options;
		config_file_options.add(common_options);

		// parse config file
		po::store(po::parse_config_file<char>("astream.conf", config_file_options), vm);
	}

	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << "Usage: astream job print [<options>] <id>" << endl << endl;
		std::cout << "Send a job to the print queue" << endl << endl;
		std::cout << printjob_desc << endl;
		return ERROR_ASTREAM_SUCCESS;
	}

	// si pas de id job fourni, on ne va pas plus loin
	if (!vm.count("id"))
	{
		return ERROR_ASTREAM_NO_JOBID_PROVIDED;
	}

	if (vm.count("range"))
	{
		string l_str_PrintRangeFormatted;
		if (!ValidatePrintRange(vm["range"].as<string>(), l_str_PrintRangeFormatted))
		{
			cout << "bad format value for --range option";
			return ERROR_ASTREAM_PRINTRANGE_BAD_FORMAT;
		}

		try
		{
			CHttpClient l_HttpClient;
			l_HttpClient.PrintJob(
				vm["server"].as<string>().c_str(),
				vm["port"].as<int>(),
				vm["id"].as<int>(),
				l_str_PrintRangeFormatted.c_str());
		}
		catch (CHttpClientException ex)
		{
			cout << ex.what() << endl;
			return 1;
		}
	}
	return ERROR_ASTREAM_SUCCESS;
}

/*********************************************************************
*
* Validation d'un PrintRange
*
**********************************************************************/
bool ValidatePrintRange(const std::string& input, std::string& output)
{
	bool l_b_BadFormat = true;
	string l_str_MinPage, l_str_MaxPage;
	if ((input.front() == '[') && (input.back() == ']'))
	{
		string l_str_Tmp = input.substr(1, input.length() - 2);
		size_t l_szt_Pos = l_str_Tmp.find(';');
		if (l_szt_Pos != string::npos)
		{
			l_str_MinPage = l_str_Tmp.substr(0, l_szt_Pos);
			l_str_MaxPage = l_str_Tmp.substr(l_szt_Pos + 1);

			if ((l_str_MinPage.find_first_not_of("0123456789") == std::string::npos) &&
				((l_str_MaxPage.find_first_not_of("0123456789") == std::string::npos) || (l_str_MaxPage.compare("-1") == 0)))
			{
				l_b_BadFormat = false;
				output = l_str_MinPage + "|" + l_str_MaxPage + ";";
			}
		}
	}
	return !l_b_BadFormat;
}
