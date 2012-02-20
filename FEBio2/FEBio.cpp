///////////////////////////////////////////////////////////////////////////////
//         ________    _________   _________     __     _________            //
//        |        |\ |        |\ |        |\   |  |\  /         \\          //
//        |    ____|| |    ____|| |    __  ||   |__|| |    ___    ||         //
//        |   |\___\| |   |\___\| |   |\_| ||    \_\| |   //  \   ||         //
//        |   ||      |   ||      |   || | ||    __   |  ||    |  ||         //
//        |   ||__    |   ||__    |   ||_| ||   |  |\ |  ||    |  ||         //
//        |       |\  |       |\  |         \\  |  || |  ||    |  ||         //
//        |    ___||  |    ___||  |    ___   || |  || |  ||    |  ||         //
//        |   |\__\|  |   |\__\|  |   |\__|  || |  || |  ||    |  ||         //
//        |   ||      |   ||      |   ||  |  || |  || |  ||    |  ||         //
//        |   ||      |   ||___   |   ||__|  || |  || |   \\__/   ||         //
//        |   ||      |        |\ |          || |  || |           ||         //
//        |___||      |________|| |__________|| |__||  \_________//          //
//                                                                           //
//      F I N I T E   E L E M E N T S   F O R   B I O M E C H A N I C S      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//
//! \mainpage FEBio Documentation
//! \section sec_intro Introduction
//! FEBio is a finite element solver that is specifically designed for three
//! dimensional biomechanical applications. It solves the nonlinear finite
//! element equations using a quasi-Newton method called the BFGS-method. It
//! also offers several biologically relevant constitutive models.
//!
//! This software is developed at the Musculoskeletal Research Laboratories
//! at the University of Utah. All rights reserved.
//! Copyright (c) 2006 - 2010
//!
//! The subversion (svn) revision number of this code can be found in the file
//! FEBio/svnrev.h
//!
//! Main developers:
//!  - Steve Maas
//!  - Jeff Weiss
//!  - Dave Rawlins
//!  - Gerard Ateshian
//!
//! Contributors:
//!  - Alexander Veress
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fem.h"
#include "FEBioLib/version.h"
#include "FEBioCommand.h"
#include "FECore/FECore.h"
#include "FEBioLib/validate.h"
#include "console.h"
#include "FEBioLib/log.h"
#include "FEBioStdSolver.h"
#include "FECore/febio.h"
#include "FEBioLib/FEPointBodyForce.h"
#include "plugin.h"

//-----------------------------------------------------------------------------
//!  Command line options

//! This structures stores the command line options that were input by the user
struct CMDOPTIONS
{
	enum {MAXFILE=512};

	bool		bdebug;			//!< debug flag

	bool	bsplash;			//!< show splash screen or not
	bool	bsilent;			//!< run FEBio in silent mode (no output to screen)

	char	szfile[MAXFILE];	//!< input file name
	char	szlog [MAXFILE];	//!< log file name
	char	szplt [MAXFILE];	//!< plot file name
	char	szdmp [MAXFILE];	//!< dump file name
	char	szcnf [MAXFILE];	//!< configuration file
	char	sztask[MAXFILE];	//!< task name
};

//-----------------------------------------------------------------------------
// forward declarations
//
bool ParseCmdLine(int argc, char* argv[], CMDOPTIONS& ops);
void Hello(FILE* fp);
int prompt(CMDOPTIONS& ops);
int get_app_path (char *pname, size_t pathsize);
extern void InitFEBioLibrary();

//-----------------------------------------------------------------------------
// we use the console to log output 
class ConsoleStream : public LogStream
{
public:
	void print(const char* sz) { printf(sz); }
};

//-----------------------------------------------------------------------------
// The starting point of the application
//
int main(int argc, char* argv[])
{
	// divert the log output to the console
	clog.SetLogStream(new ConsoleStream);

	// parse the command line
	CMDOPTIONS ops;
	if (ParseCmdLine(argc, argv, ops) == false) return 0;

	// load the license file
	LoadLicenseFile();

	// print welcome message
	if (ops.bsplash && (!ops.bsilent)) Hello(stdout);

	// if silent mode only output to file
	if (ops.bsilent) clog.SetMode(Logfile::FILE_ONLY);

	// initialize FEBio library
	InitFEBioLibrary();

	// if there are no arguments, print the FEBio prompt
	if (argc == 1)
	{
		 if (prompt(ops) == 0) return 0;
	}

	// create the one and only FEM object
	FEM fem;

	// intialize the framework
	FEBioCommand::SetFEM(&fem);

	// read the configration file if specified
	if (ops.szcnf[0])
	{
		if (fem.Configure(ops.szcnf) == false) return 1;
	}

	// store the input file name
	fem.SetInputFilename(ops.szfile);

	// set the output filenames
	fem.SetLogFilename (ops.szlog);
	fem.SetPlotFilename(ops.szplt);
	fem.SetDumpFilename(ops.szdmp);

	// set options that were passed on the command line
	fem.SetDebugFlag(ops.bdebug);

	// get the FEBio kernel
	FEBioKernel& febio = FEBioKernel::GetInstance();

	// find a task
	FEBioTask* ptask = febio.Create<FEBioTask>(ops.sztask, &fem);
	if (ptask == 0)
	{
		fprintf(stderr, "Don't know how to do task: %s\n", ops.sztask);
		int N = febio.Count<FEBioTask>();
		fprintf(stderr, "Nr. of task defined = %d:\n", N);
		for (int i=0; i<N; ++i)
		{
			const char* sz = febio.GetTypeStr<FEBioTask>(i);
			assert(sz);
			if (sz == 0) sz = "(Unknown)";
			fprintf(stderr, "Task %d = %s\n", i+1, sz);
		}
		return 1;
	}

	// run the FEBio analysis
	bool bret = ptask->Run(ops.szfile);

	// return the error code of the run
	return (bret?0:1);
}

//-----------------------------------------------------------------------------
//!  Parses the command line and returns a CMDOPTIONS structure
//
bool ParseCmdLine(int nargs, char* argv[], CMDOPTIONS& ops)
{
	// set default options
	ops.bdebug = false;
	ops.bsplash = true;
	ops.bsilent = false;

	bool blog = false;
	bool bplt = false;
	bool bdmp = false;
	bool brun = true;

	ops.szfile[0] = 0;
	ops.sztask[0] = 0;

	// set the location of the configuration file
	char szpath[1024] = {0};
	get_app_path (szpath, 1023);

	char* ch = strrchr(szpath, '\\');
	if (ch == 0) ch = strrchr(szpath, '/');
	if (ch) ch[1] = 0;

	sprintf(ops.szcnf, "%sfebio.xml", szpath);

	// loop over the arguments
	char* sz;
	for (int i=1; i<nargs; ++i)
	{
		sz = argv[i];

		if (strcmp(sz,"-r") == 0)
		{
			if (ops.sztask[0] != 0) { fprintf(stderr, "-r is incompatible with other command line option.\n"); return false; }
			strcpy(ops.sztask, "restart");
			strcpy(ops.szfile, argv[++i]);
		}
		else if (strcmp(sz, "-d") == 0)
		{
			if (ops.sztask[0] != 0) { fprintf(stderr, "-d is incompatible with other command line option.\n"); return false; }
			strcpy(ops.sztask, "diagnose");
			strcpy(ops.szfile, argv[++i]);
		}
		else if (strcmp(sz, "-p") == 0)
		{
			bplt = true;
			strcpy(ops.szplt, argv[++i]);
		}
		else if (strcmp(sz, "-a") == 0)
		{
			bdmp = true;
			strcpy(ops.szdmp, argv[++i]);
		}
		else if (strcmp(sz, "-o") == 0)
		{
			blog = true;
			strcpy(ops.szlog, argv[++i]);
		}
		else if (strcmp(sz, "-i") == 0)
		{
			strcpy(ops.szfile, argv[++i]);
		}
		else if (strcmp(sz, "-s") == 0)
		{
			if (ops.sztask[0] != 0) { fprintf(stderr, "-s is incompatible with other command line option.\n"); return false; }
			strcpy(ops.sztask, "optimize");
			strcpy(ops.szfile, argv[++i]);
		}
		else if (strcmp(sz, "-g") == 0)
		{
			ops.bdebug = true;
		}
		else if (strcmp(sz, "-nosplash") == 0)
		{
			// don't show the welcome message
			ops.bsplash = false;
		}
		else if (strcmp(sz, "-silent") == 0)
		{
			// no output to screen
			ops.bsilent = true;
		}
		else if (strcmp(sz, "-cnf") == 0)
		{
			strcpy(ops.szcnf, argv[++i]);
		}
		else if (strcmp(sz, "-noconfig") == 0)
		{
			ops.szcnf[0] = 0;
		}
		else if (strncmp(sz, "-task", 5) == 0)
		{
			if (sz[5] != '=') { fprintf(stderr, "command line error when parsing task\n"); return false; }
			strcpy(ops.sztask, sz+6);
		}
		else if (strcmp(sz, "-info")==0)
		{
			FILE* fp = stdout;
			if ((i<nargs-1) && (argv[i+1][0] != '-'))
			{
				fp = fopen(argv[++i], "wt");
				if (fp == 0) fp = stdout;
			}
			fprintf(fp, "compiled on " __DATE__ "\n");
			fprintf(fp, "FEBio version  = %d.%d.%d\n", VERSION, SUBVERSION, SUBSUBVERSION);
			fprintf(fp, "FECore version = %s\n", FECore::get_version_string());
			fprintf(fp, "SVN revision   = %d\n", SVNREVISION);
			if (fp != stdout) fclose(fp);
		}
		else if (strcmp(sz, "-norun") == 0)
		{
			brun = false;
		}
		else if (strcmp(sz, "-import") == 0)
		{
			char* szfile = argv[++i];
			if (LoadPlugin(szfile) == false)
			{
				fprintf(stderr, "Failed loading plugin %s\n\n", szfile);
			}
			else
			{
				fprintf(stderr, "Success loading plugin %s\n\n", szfile);
			}
		}
		else
		{
			// we allow FEBio to run without a -i option
			// so that we can run an .feb file by right-clicking on it in windows
			if (nargs == 2)
			{
				char* c = strrchr(argv[1], '.');
				if (c && (strcmp(c, ".feb")==0))
				{
					strcpy(ops.szfile, argv[1]);
				}
				else
				{
					fprintf(stderr, "FATAL ERROR: Invalid command line option\n\n");
					return false;
				}
			}
			else
			{
				fprintf(stderr, "FATAL ERROR: Invalid command line option\n\n");
				return false;
			}
		}
	}

	// if no task is defined, we assume a std solve is wanted
	if (ops.sztask[0] == 0) strcpy(ops.sztask, "solve");

	// derive the other filenames
	char szbase[256]; strcpy(szbase, ops.szfile);
	ch = strrchr(szbase, '.');
	if (ch) *ch = 0;

	char* szext = (ch?ch+1:0);

	strcpy(szpath, ops.szfile);
	ch = strrchr(szpath, '/');
	if (ch == 0) ch = strrchr(szpath, '\\');
	if (ch) *(ch+1) = 0; else szpath[0] = 0;

	if (szext && ((strcmp(szext, "feb")==0) ||
				  (strcmp(szext, "xml")==0) ||
				  (strcmp(szext, "XML")==0)))
	{
		if (!blog) sprintf(ops.szlog, "%s.log", szbase);
		if (!bplt) sprintf(ops.szplt, "%s.plt", szbase);
		if (!bdmp) sprintf(ops.szdmp, "%s.dmp", szbase);
	}
	else
	{
		if (!blog) sprintf(ops.szlog, "%sf3log.txt", szpath);
		if (!bplt) sprintf(ops.szplt, "%sf3plot"   , szpath);
		if (!bdmp) sprintf(ops.szdmp, "%sf3dmp"    , szpath);
	}

	return brun;
}

//-----------------------------------------------------------------------------
//! Prints the FEBio prompt. If the user did not enter anything on the command
//! line when running FEBio then commands can be entered at the FEBio prompt.
//! This function returns the command arguments as a CMDOPTIONS structure.
int prompt(CMDOPTIONS& ops)
{
	// get a pointer to the console window
	Console* pShell = Console::GetHandle();

	int nargs;
	char* argv[32];

	fprintf(stderr, "Type help for an overview of commands.\n");

	while (1)
	{
		// get a command from the shell
		pShell->GetCommand(nargs, argv);
		if (nargs > 0)
		{
			if (strcmp(argv[0], "quit") == 0) return 0;
			else if (strcmp(argv[0], "help") == 0)
			{
				fprintf(stderr, "\n");
				fprintf(stderr, "help - print this info\n");
				fprintf(stderr, "quit - exits the application\n");
				fprintf(stderr, "run [-i,-s] <file> [OPTIONS] - run an FEBio input file\n");
				fprintf(stderr, "version - print version information\n");
			}
			else if (strcmp(argv[0], "run") == 0)
			{
				ParseCmdLine(nargs, argv, ops);
				return 1;
			}
			else if (strcmp(argv[0], "version") == 0)
			{
				fprintf(stderr, "\nFEBio version %d.%d.%d\n", VERSION, SUBVERSION, SUBSUBVERSION);
				fprintf(stderr, "compiled on " __DATE__ "\n");
				fprintf(stderr, "using FECore version %s\n\n", FECore::get_version_string());
			}
			else
			{
				printf("Unknown command: %s\n", argv[0]);
			}
		}
	}
	return 0;
}
