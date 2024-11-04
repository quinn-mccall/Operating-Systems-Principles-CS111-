#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	// Base cases
	if (argc == 1) // no commands
	{
		errno = EINVAL;
		return (errno);
	}
	if (argc == 2) // one command
	{
		if (execlp(argv[1], argv[1], NULL) == -1)
		{
			perror("execution failed on argument 1");
			exit(errno);
		}
	}

	// More than one command
	int fd[2];
	int prevfd = -1; // Saves the file descriptor for the next process to read from

	for (int i = 1; i < argc; i++)
	{

		if (i < argc - 1)
		{
			if (pipe(fd) == -1)
			{
				perror("pipe failed");
				return 1;
			}
		}

		int pid = fork();

		if (pid < 0)
		{
			perror("fork failed");
			return 2;
		}

		if (pid == 0)
		{
			if (i > 1)
			{
				dup2(prevfd, STDIN_FILENO);
				close(prevfd);
			}
			if (i < argc - 1)
			{
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
			}
			close(fd[1]);
			close(fd[0]);

			if (execlp(argv[i], argv[i], NULL) == -1) // Execute the command at position i
			{
				perror("execution failed");
				exit(errno);
			}
		}

		if (prevfd != -1)
		{
			close(prevfd);
		}
		if (i < argc - 1)
		{
			prevfd = fd[0];
		}
		close(fd[1]);
		int status;
		waitpid(pid, &status, 0); // waiting to avoid orphaned processes

		if (WIFEXITED(status))
		{
			int child_exit_status = WEXITSTATUS(status);
			if (child_exit_status != 0)
			{
				return child_exit_status;
			} // Return child's exit status
		}
	}
	return 0;
}
