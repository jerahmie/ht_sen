#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <pwd.h>
#include <getopt.h>
#include <time.h>
#include "rpi_gpio.h"

void print_help(void)
{
  printf("This is the help message.\n");
  exit(EXIT_SUCCESS);
}


static void setup_daemon(const char *logdir)
{
  
  /* Fork off the parent process */
  pid_t pID = fork();

  /* An error occurred */
  if (pID < 0)
    {
      printf("Failed to fork.\n");
      exit(EXIT_FAILURE);
    }

  /* Success: let the parent terminate */
  else if (pID > 0)
    {
      exit(EXIT_SUCCESS);
    }
  /* Allow the child process to continue. */
  if (pID == 0)
    {
      // child process 
      if (setsid() < 0)
	exit(EXIT_FAILURE);
    }
  /* Catch, ignore, and handle signals */
  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  /* Fork off for the second time */
  pID = fork();

  /* An error occurred */
  if (pID < 0 )
    {
      exit(EXIT_FAILURE);
    }

  /* Success: let the parent process terminate */
  if (pID > 0)
    {
      exit(EXIT_SUCCESS);
    }

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the log directory */
  struct stat st = {0};
  if (stat(logdir, &st ) == -1)
  {
    syslog(LOG_ERR, "Log directory %s does not exits.", logdir);
    exit(EXIT_FAILURE);
  }
  else
    {
      chdir(logdir);
    }

  /* Close standard file descriptors */

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* Open the log file. */
  
  openlog("firstdaemon", LOG_PID, LOG_DAEMON);

}

int main(int argc, char* argv[])
{
  FILE *fp = NULL;
  int iteration = 0;

  // TODO: add parameter to save log to directory
  // in the meantime, use the current user home directory (this can be default).
  struct passwd *pw = getpwuid(getuid());
  char *logdir = pw->pw_dir;
  time_t timer;
  char time_buffer[25];
  struct tm* tm_info;
  
  int c;
  
  while (1)
    {
      /* Define the command line options */
      static struct option long_options[] =
	{
	  { "help",  no_argument, 0, 'h' },
	  { "file",  required_argument, 0, 'f' },
	  {0, 0, 0, 0}
	};

      /* getopt_long stores the option index here. */
      int option_index = 0;
  
      c = getopt_long(argc, argv, "hf:",
		       long_options, &option_index);
      if (c == -1)
	{
	  break;
	}

      switch(c)
	{
	case 'h':
	  print_help();
	  break;

	case 'f':
	  logdir = optarg;
	  printf("File location: %s\n", optarg);
	  break;
	  
	default:
	  abort();
	}
  
    }
  
  /* Check for log directory location and create, if necessary. */
  struct stat st = {0};
  
  if (stat(logdir, &st) == -1)
    {
      if (mkdir(logdir, 0700) == -1)
	{
	  syslog(LOG_ERR, "Could not create directory %s.", logdir);
	}
    }
  
  setup_daemon((const char*)logdir);

  /* Open the output file */
  syslog(LOG_NOTICE, "Opening log file.");
  fp = fopen("Log.txt", "a+");
  if (fp == NULL)
    {
      syslog(LOG_NOTICE, "Failed to open log file.");
      return(EXIT_FAILURE);
    }

  // configure GPIO
  setup_rpi_gpio();

  while (1)
    {
      // TODO: Insert daemon code here.
      time(&timer);
      tm_info = localtime(&timer);
      strftime(time_buffer, 25, "%Y:%m:%d:%H:%M:%S", tm_info);
      
      iteration++;
      fprintf(fp, "%s, Loop iteration %d\n", time_buffer, iteration);
      fflush(fp);
      sleep(10);
      //      break;
    }

  syslog(LOG_NOTICE, "First daemon terminated.");
  closelog();

  /* Close the output file */
  fclose(fp);

  return EXIT_SUCCESS;
}
