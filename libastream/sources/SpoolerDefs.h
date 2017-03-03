#pragma once

#include <boost/optional.hpp>
#include <ctime>
#include <string>
using namespace std;

typedef enum
{
    Uploading = 0,
    UploadingError = 1,
    Analyzing = 2,
    AnalyzingError = 3,
    WaitingToHash = 4,
    Hashing = 5,
    WaitingForOperator = 6,
    WaitingForParams = 7,
    Waiting = 8,
    WaitingForPSToPDFConverting = 9,
    PSToPDFConverting = 10,
    PSToPDFConvertionError = 11,
    PSToPDFConverted = 12,
    WaitingForPDFToAFPConverting = 13,
    PDFToAFPConverting = 14,
    PDFToAFPConvertionError = 15,
    PDFToAFPConverted = 16,
    WaitingForIndexing = 17,
    Indexing = 18,
    IndexingError = 19,
    Indexed = 20
} EPreparingStatus;

typedef enum
{
    WaitingForAFPToIPDSConverting = 0,
    AFPToIPDSQueued = 1,
    AFPToIPDSConverting = 2,
    AFPToIPDSConverted = 3,
    AFPToIPDSConvertionError = 4,
    WaitingForPrinterResuming = 5,
    WaitingForOperatorResuming = 6
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
	AskOperatorRecoveryPage = 7,
    PrintingAborted = 8
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
