/**
 *       @file  daemonize.cc
 *      @brief  A simple routine to daemonize the calling process
 *
 * The following program provides a basic daemon with the following features:
 * - Logs messages to the system log (via syslog).
 * - Creates a lock file to prevent the daemon from being run twice.
 * - Changes the effective user (drops privileges).
 * - Startup errors are reported to the main process.
 * Partially based on code of Doug Potter:
 * http://www.itp.uzh.ch/~dpotter/howto/daemonize
 * and inspired by the daemonize tool by Brian M. Clapper:
 * http://software.clapper.org/daemonize/
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
#include <sys/file.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>

/* A global variable to signal if we are running as a daemon */
unsigned char daemonized = 0;

static void child_handler(int signum) {
	switch(signum) {
	case SIGALRM: exit(EXIT_FAILURE); break;
	case SIGUSR1: exit(EXIT_SUCCESS); break;
	case SIGCHLD: exit(EXIT_FAILURE); break;
	}
}

/*
 * Switch the process's effective and real user IDs to the ID associated with
 * the specified user name. Also switches to that user's group ID to the
 * primary group associated with the user.
 *
 * This function returns 0 on success, a negative number if anything fails.
 *
 * Parameters:
 *     username - name of user to which to switch
 *     pidfile  - if non-NULL, specifies PID file path, ownership of which
 *                 should be changed to the user */
static int switch_user(const char *username, const char *pidfile) {
	uid_t uid = getuid();
	struct passwd *pw;

	/* Lookup for the specified username */
	pw = getpwnam(username);
	if (pw == NULL) {
		syslog(LOG_ERR, "Can't find user [%s] in password file",
				username);
		return -1;
	}

	if (uid == pw->pw_uid) {
		syslog(LOG_NOTICE, "Already running as user [%s]",
					pw->pw_name);
		return 0;
	}

	if (uid != 0) {
		syslog(LOG_ERR, "Must be root to run as a different [%s] user",
				pw->pw_name);
		return -2;
	}

	if (setgid(pw->pw_gid) != 0) {
		syslog(LOG_ERR, "Can't set GID to [%d] (Error: %d, %s)",
				pw->pw_gid, errno, strerror(errno));
		return -3;
	}

	/*
	 *  For systems supporting multiple group memberships, make sure ALL
	 *  groups are added to the process, just in case someone depends on them.
	 *  Patch by Ken Farnen <kenf@14Cubed.COM>, 18 February 2010. Modified
	 *  to put the #ifdef in the config.h header, rather than in here. */
	if (initgroups(pw->pw_name, pw->pw_gid) == -1) {
		syslog(LOG_ERR, "Can't initialize secondary groups for [%s] (Error %d, %s)",
				pw->pw_name, errno, strerror(errno));
		return -4;
	}

	if (pidfile != NULL) {
		syslog(LOG_NOTICE, "Changing ownership of PID file to [%d:%s]",
				pw->pw_uid, username);
		if (chown(pidfile, pw->pw_uid, pw->pw_gid) == -1) {
			syslog(LOG_ERR, "Can't change owner of PID file [%s] to [%d:%s] (Error %d, %s)",
					pidfile, pw->pw_uid, username,
					errno, strerror(errno));
			return -5;
		}
	}

	if (setegid(pw->pw_gid) != 0) {
		syslog(LOG_ERR, "Can't set egid to [%d] (Error %d, %s)",
				pw->pw_gid, errno, strerror(errno));
		return -6;
	}

	if (setuid(pw->pw_uid) != 0) {
		syslog(LOG_ERR, "Can't set uid to [%d] (Error %d, %s)",
				pw->pw_uid, errno, strerror(errno));
		return -7;
	}

	if (seteuid(pw->pw_uid) != 0) {
		syslog(LOG_ERR, "Can't set euid to [%d] (Error %d, %s)",
				pw->pw_uid, errno, strerror(errno));
		return -8;
	}

	/*
	 * Initialize environment to match new username.
	 * Patch by Ken Farnen <kenf@14Cubed.COM>, 18 February 2010. */
	setenv("USER", pw->pw_name, 1);
	setenv("LOGNAME", pw->pw_name, 1);
	setenv("HOME", pw->pw_dir, 1);

	return 0;
}

int daemonize(const char *name, const char *uid,
		const char *lockfile, const char *pidfile,
		const char *rundir) {
	pid_t pid, sid, parent;
	int lfd = -1; // Lockfile descriptor
	int pfd = -1; // PID file descriptor

	/* already a daemon */
	if (getppid() == 1)
		return 0;

	/* Create the lock file as the current user */
	if (lockfile && lockfile[0]) {
		syslog(LOG_NOTICE, "Writing lockfile to [%s]", lockfile);
		lfd = open(lockfile, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
		if (lfd < 0) {
			syslog(LOG_ERR, "unable to create lockfile [%s] (Error: %d, %s)",
					lockfile, errno, strerror(errno) );
			return -1;
		}
		if (flock(lfd, LOCK_EX|LOCK_NB) != 0) {
			syslog(LOG_ERR, "unable to lock the lockfile [%s] (Error: %d, %s)",
					lockfile, errno, strerror(errno));
			return -1;
		}
	}

	/* Create the PID file */
	if (pidfile && pidfile[0]) {
		pfd = open(pidfile, O_CREAT|O_WRONLY,
				S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (pfd < 0) {
			syslog(LOG_ERR, "unable to create PID file [%s] (Error: %d, %s)",
					pidfile, errno, strerror(errno));
			return -2;
		}
		close(pfd);
	}

	/* Switch user */
	if (switch_user(uid, pidfile) != 0) {
		return -3;
	}

	/* Change the current working directory.
	 * This prevents the current directory from being locked; hence not
	 * being able to remove it. */
	if (chdir(rundir) != 0) {
		syslog(LOG_ERR, "unable to change directory to [%s] (Error: %d, %s)",
				rundir, errno, strerror(errno) );
		return -4;
	}

	/* Redirect standard files to /dev/null */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

	/* Trap signals that we expect to recieve */
	signal(SIGCHLD, child_handler);
	signal(SIGUSR1, child_handler);
	signal(SIGALRM, child_handler);

/*******************************************************************************
 *      PROCESS DAEMONIZATION
 ******************************************************************************/

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		syslog(LOG_ERR, "unable to fork daemon (Error: %d, %s)",
				errno, strerror(errno));
		return -5;
	}

	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		/* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
		   for two seconds to elapse (SIGALRM). */
		alarm(2);
		/* The pause() function shall suspend the calling thread until
		 * delivery of a signal whose action is either to execute a
		 * signal-catching function or to terminate the process.
		 * If the action is to terminate the process, pause() shall not
		 * return. */
		 pause();
		 exit(EXIT_FAILURE);
	}

	/* At this point we are executing as the child process */
	parent = getppid();

	/*
	 * Keep track of the daemon PID */
	if (pidfile && pidfile[0]) {
		FILE  *fPid = NULL;

		syslog(LOG_NOTICE, "Writing process ID to [%s]", pidfile);
		if ((fPid = fopen(pidfile, "w")) == NULL) {
			syslog(LOG_ERR, "Can't open PID file [%s] (Error %d, %s)",
					pidfile, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		fprintf(fPid, "%d\n", getpid());
		fclose(fPid);
	}

	/* Cancel certain signals */
	signal(SIGCHLD, SIG_DFL); /* A child process dies */
	signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP,  SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM, SIG_DFL); /* Die on SIGTERM */

	/* Clear any inherited file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "unable to create a new session (Error: %d, %s)",
				errno, strerror(errno));
		exit(EXIT_FAILURE);
	}


	/*
	 * Make sure we have a relatively sane environment
	 * Patch by Ken Farnen <kenf@14Cubed.COM>, 18 February 2010. */
	if (getenv("IFS") == NULL)
		setenv("IFS"," \t\n",1);
	if (getenv("PATH") == NULL)
		setenv("PATH","/usr/local/sbin:/sbin:/bin:/usr/sbin:/usr/bin", 1);

	/* Set the daemon process name */
	if (prctl(PR_SET_NAME, name, 0, 0, 0) != 0) {
		syslog(LOG_ERR, "unable to set daemon name [%s] (Error: %d, %s)",
			name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Mark this process as running as a daemon */
	daemonized = 1;

	/* Tell the parent process that we are A-okay */
	kill(parent, SIGUSR1);

	return 0;
}
