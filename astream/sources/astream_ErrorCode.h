#pragma once

typedef enum
{
	ERROR_ASTREAM_SUCCESS,								// pas d'erreur
	ERROR_ASTREAM_UNKNOWN_COMMAND,					// commande inconnue
	ERROR_ASTREAM_REQUIRED_OPTION_MISSING,			// option obligatoire manquante
	ERROR_ASTREAM_UNKNOWN_OPTION,						// option inconnue
	ERROR_ASTREAM_NO_INPUT_FILE_PROVIDED,			// aucun fichier d'entrée fourni
	ERROR_ASTREAM_NO_JOBID_PROVIDED,					// id job non fourni
	ERROR_ASTREAM_JOB_UPLOAD_FAILED,					// le job n'a pas pu être soumis (problème d'upload)
	ERROR_ASTREAM_PRINTRANGE_BAD_FORMAT,			// mauvais format pour le printRange
	ERROR_ASTREAM_GET_JOB_FAILED,						// erreur lors de la récupération des infos sur le job
	ERROR_ASTREAM_ADD_JOB_TO_PRINTQUEUE_FAILED,	// le job n'a pas pu être ajouté à la print queue
};