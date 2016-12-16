#pragma once

#include <boost/optional.hpp>
#include <ctime>
#include <string>
using namespace std;

typedef enum
{
	Uploading = 0,
	Waiting = 1,
	WaitingForPDFToAFPConverting = 2,
	PDFToAFPConverting = 3,
	PDFToAFPConverted = 4,
	PDFToAFPConvertionError = 5,
	WaitingForIndexing = 6,
	Indexing = 7,
	Indexed = 8,
	IndexingError = 9,
	UploadingError = 10,
	Analyzing = 11,
	AnalyzingError = 12,
	WaitingForParams = 13,
	WaitingForPSToPDFConverting = 14,
	PSToPDFConverting = 15,
	PSToPDFConverted = 16,
	PSToPDFConvertionError = 17
} EPreparingStatus;

typedef enum
{
	WaitingForAFPToIPDSConverting = 0,
	AFPToIPDSConverting = 1,
	AFPToIPDSConverted = 2,
	AFPToIPDSConvertionError = 3,
	WaitingForPrinterResuming = 4,
	WaitingForOperatorResuming = 5
}EConvertionStatus;

typedef enum
{
	NotReady = 0,
	ReadyToPrint = 1,
	WaitingForPrinting = 2,
	Printing = 3,
	Printed = 4,
	PrintingError = 5,
	ResumingPrintingError = 6,
	AskOperatorRecoveryPage = 7
}EPrintingStatus;

typedef struct
{
	int m_i_JobId;
	string m_str_OriginalDocName;
	string m_str_UserFriendlyName;
	boost::optional<string> m_str_Template;
	boost::optional<string> m_str_TemplateName;
	boost::optional<string> m_str_FormDef;
	EPreparingStatus m_EPreparingStatus;
	//time_t m_AddTime;
}S_JobInfos;

/////////////////////////////////////////////
//   JSON Job sample
/////////////////////////////////////////////
//{
//	"AddTime":"\/Date(1481626220556+0100)\/",
//		"DocType" : 1,
//		"FormDef" : null,
//		"IsProcessingIndexing" : 0,
//		"IsProcessingPdf2Afp" : 0,
//		"IsProcessingPs2Pdf" : 0,
//		"LastPrintJobStatus" : null,
//		"NbPDFPages" : null,
//		"NbPages" : null,
//		"NbUps" : null,
//		"OriginalDocName" : "C++11_smart_ptrs.pdf",
//		"PreparingProgress" : 0,
//		"PreparingStatus" : 8,
//		"ProcessingDocName" : null,
//		"RetentionPeriod" : 0,
//		"Template" : null,
//		"TemplateName" : null, 
//		"UserFriendlyName" : "C++11_smart_ptrs.pdf", 
//		"id" : 1, 
//		"idJobSetting" : null, 
//		"idPrintSetting" : null
//}
