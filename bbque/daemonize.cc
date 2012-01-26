/**
 *       @file  daemonize.cc
 *      @brief  A simple routine to daemonize the calling process
 *
 * The following program provides a basic daemon with the following features:
 * - Logs messages to the system log (via syslog).
 * - Creates a lock file to prevent the daemon from being run twice.
 * - Changes the effective user (drops privileges).
 * - Startup errors are reported to the main process.
 * Based on code of Doug Potter:
 * http://www.itp.uzh.ch/~dpotter/howto/daemonize
 *
 *     @author  Patrick Bellasi (derkling), derkling@gmail.com
 *
 *   @internal
 *     Created  25/01/2012
 *    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
 *    Compiler  gcc/g++
 *     Company  Politecnico di Milano
 *   Copyright  Copyright (c) 2012, Patrick Bellasi
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License as published by the Free Software Foundation.
 * ============================================================================
 */

#include "bbque/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>

/* A global variable to signal if we are running as a daemon */
unsigned char daemonized = 0;

static void child_handler(int signum) {
	switch(signum) {
	case SIGALRM: exit(EXIT_FAILURE); break;
	case SIGUSR1: exit(EXIT_SUCCESS); break;
	case SIGCHLD: exit(EXIT_FAILURE); break;
	}
}

int daemonize(const char *name, const char *uid, const char *gid,
		const char *lockfile, const char *rundir) {
	pid_t pid, sid, parent;
	int lfp = -1;

	/* already a daemon */
	if (getppid() == 1)
		return 0;

	/* Create the lock file as the current user */
	if (lockfile && lockfile[0]) {
		lfp = ::open(lockfile, O_RDWR|O_CREAT, 0640);
		if (lfp < 0) {
			syslog(LOG_ERR, "unable to create lockfile [%s] (Error: %d, %s)",
					lockfile, errno, strerror(errno) );
			return -1;
		}
	}

	/* Drop user if there is one, and we were run as root */
	if (getuid() == 0 || geteuid() == 0) {
		struct passwd *pw = getpwnam(uid);
		if (pw) {
			syslog(LOG_NOTICE, "setting user to [%s]", uid);
			setuid(pw->pw_uid);
		}
	}

	/* Trap signals that we expect to recieve */
	signal(SIGCHLD, child_handler);
	signal(SIGUSR1, child_handler);
	signal(SIGALRM, child_handler);

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		syslog(LOG_ERR, "unable to fork daemon (Error: %d, %s)",
				errno, strerror(errno));
		return -2;
	}

	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		/* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
		   for two seconds to elapse (SIGALRM).  pause() should not return. */
		alarm(2);
		pause();

		exit(EXIT_FAILURE);
	}

	/* At this point we are executing as the child process */
	parent = getppid();

	/* Cancel certain signals */
	signal(SIGCHLD, SIG_DFL); /* A child process dies */
	signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP,  SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM, SIG_DFL); /* Die on SIGTERM */

	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "unable to create a new session (Error: %d, %s)",
				errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory.  This prevents the current
	   directory from being locked; hence not being able to remove it. */
	if ((chdir(rundir)) < 0) {
		syslog(LOG_ERR, "unable to change directory to [%s] (Error: %d, %s)",
				rundir, errno, strerror(errno) );
		exit(EXIT_FAILURE);
	}

	/* Redirect standard files to /dev/null */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

	/* Set the daemon process name */
	if (prctl(PR_SET_NAME, name, 0, 0, 0) != 0) {
		syslog(LOG_ERR, "unable to set daemon name [%s] (Error: %d, %s)",
			name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Tell the parent process that we are A-okay */
	kill(parent, SIGUSR1);

	/* Mark this process as running as a daemon */
	daemonized = 1;

	return 0;
}
