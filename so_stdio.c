	#define CONFIG_FORTIFY_SOURCE

	#include "so_stdio.h"
	#include <fcntl.h>
	#include <string.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>


SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *file = malloc(sizeof(SO_FILE));

	if (file == NULL) {
		free(file);
		return NULL;
	}
	file->toBeWritten = !flagTBRTrue;
	file->errorFound = flagNotError;
	file->bytesRead = 0;
	file->fcursor = 0;
	file->openedPipe = 0;
	file->eof = 0;
	file->pid = 0;
	if (strcmp(mode, "r") == 0) {
		file->fd = open(pathname, O_RDONLY);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}
		file->cursor = 0;
		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->permision[0] = 'r';
		file->permision[1] = '\0';
		return file;
	}

	if (strcmp(mode, "w") == 0) {
		file->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}
		file->cursor = 0;
		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->permision[0] = 'w';
		file->permision[1] = '\0';
		return file;
	}

	if (strcmp(mode, "a") == 0) {
		file->fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}
		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->cursor = 0;
		file->permision[0] = 'a';
		file->permision[1] = '\0';
		return file;
	}

	if (strcmp(mode, "r+") == 0) {
		file->fd = open(pathname, O_RDWR);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}
		file->cursor = 0;
		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->permision[0] = 'r';
		file->permision[1] = '+';
		return file;
	}

	if (strcmp(mode, "w+") == 0) {
		file->fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}
		file->cursor = 0;
		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->permision[0] = 'w';
		file->permision[1] = '+';
		return file;
	}

	if (strcmp(mode, "a+") == 0) {
		file->fd = open(pathname, O_RDWR | O_CREAT | O_APPEND, 0644);
		if (file->fd < 0) {
			free(file);
			return NULL;
		}

		for (int i = 0; i < bufferSize; i++)
			file->buffer[i] = '\0';
		file->cursor = 0;
		file->permision[0] = 'a';
		file->permision[1] = '+';
		return file;
	}

	free(file);
	return NULL;
	}

int so_fclose(SO_FILE *stream)
{
	so_fflush(stream);

	for (int i = 0; i < bufferSize; i++)
		stream->buffer[i] = '\0';
	stream->cursor = 0;
	stream->pid = 0;
	stream->bytesRead = 0;
	stream->toBeWritten = 0;
	stream->openedPipe = 0;
	stream->fcursor = 0;
	int aux = stream->errorFound;

	stream->errorFound = 0;
	char w = stream->permision[0];

	stream->permision[0] = '\0';
	stream->permision[1] = '\0';
	if (close(stream->fd) == 0) {
		stream->fd = 0;
		free(stream);
		if (w != 'r')
			return aux;

		return 0;
	}
	stream->fd = 0;
	free(stream);
	return SO_EOF;
}

int so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	so_fflush(stream);
	stream->fcursor = lseek(stream->fd, offset, whence);
	if (stream->fcursor < 0) {
		stream->errorFound = flagError;
		return -1;
	}
	return 0;
}

long so_ftell(SO_FILE *stream)
{
	return stream->fcursor;
}

int so_fflush(SO_FILE *stream)
{
	if (stream->permision[0] == 'a')
		lseek(stream->fd, 0, SEEK_END);
	if (strcmp(stream->permision, "r") != 0)
		if (stream->toBeWritten == flagTBRTrue) {
			int totalBytesWritten = 0, currentBytesWritten = 0;

			while (totalBytesWritten != stream->cursor) {
				currentBytesWritten = write(stream->fd, stream->buffer+totalBytesWritten, stream->cursor-totalBytesWritten);
				if (currentBytesWritten == -1) {
					stream->errorFound = flagError;
					return SO_EOF;
				}
				totalBytesWritten += currentBytesWritten;
			}
			stream->toBeWritten = !flagTBRTrue;
		}

	for (int i = 0; i < stream->cursor; i++)
		stream->buffer[i] = '\0';
	stream->cursor = 0;
	stream->bytesRead = 0;
	return 0;
}

int so_fgetc(SO_FILE *stream)
{
	if (strcmp(stream->permision, "w") == 0 || strcmp(stream->permision, "a") == 0) {
		stream->errorFound = flagError;
		return SO_EOF;
	}

	if (stream->cursor == 0) {
		stream->bytesRead = read(stream->fd, stream->buffer, bufferSize);
		if (stream->bytesRead < 0) {
			stream->errorFound = flagError;
			return SO_EOF;
		}
		if (stream->bytesRead == 0) {
			stream->eof = 1;
			return SO_EOF;
		}
		stream->cursor = 0;
	}

	int aux = (int)(stream->buffer[stream->cursor]);

	stream->cursor++;

	if (stream->cursor == stream->bytesRead)
		so_fflush(stream);
	stream->fcursor++;
	return aux;
}

int so_fputc(int c, SO_FILE *stream)
{
	if (strcmp(stream->permision, "r") == 0) {
		stream->errorFound = flagError;
		return SO_EOF;
	}

	if (stream->cursor > bufferSize || stream->cursor < 0) {
		stream->errorFound = flagError;
		return SO_EOF;
	}

	if (stream->cursor == bufferSize) {
		so_fflush(stream);
		stream->cursor = 0;
	}
	stream->buffer[stream->cursor] = (char)c;

	int aux = (int)(stream->buffer[stream->cursor]);

	stream->cursor++;
	stream->toBeWritten = flagTBRTrue;
	stream->fcursor++;
	return aux;
}

int so_feof(SO_FILE *stream)
{
	return stream->eof;
}

int so_ferror(SO_FILE *stream)
{
	return stream->errorFound;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int elementsRead = 0;

	if (ptr == NULL) {
		stream->errorFound = flagError;
		return 0;
	}
	for (elementsRead = 0; elementsRead < nmemb; elementsRead++) {
		for (int j = 0; j < size; j++) {
			int buf = so_fgetc(stream);

			if (buf == SO_EOF) {
				stream->errorFound = flagError;
				return elementsRead;
			}
			*(unsigned char *)ptr = (unsigned char)buf;
			ptr++;
	}
	}

	return elementsRead;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{

	int elementeScrise = 0, k = 0;
	unsigned char *aux = (unsigned char *)ptr;

	for (int i = 0; i < nmemb; i++) {
		for (int j = 0; j < size; j++) {
			int nr = (int)(aux[k++]);

			if (so_fputc(nr, stream) == -1) {
				stream->errorFound = flagError;
				return elementeScrise;
			}
		}
		elementeScrise++;
	}

	return elementeScrise;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	if (command == NULL || type == NULL)
		return NULL;
	int pipe_fd[2];

	if (pipe(pipe_fd) == -1)
		return NULL;

	SO_FILE *file = malloc(sizeof(SO_FILE));

	file->bytesRead = 0;
	file->cursor = 0;
	file->fcursor = 0;
	file->eof = 0;
	file->errorFound = 0;
	file->fd = pipe_fd[0];
	file->toBeWritten = !flagTBRTrue;

	for (int i = 0; i < bufferSize; i++)
		file->buffer[i] = '\0';

	if (strcmp(type, "r") == 0) {
		file->permision[0] = 'r';
		file->permision[1] = '\0';
		pid_t pid = fork();

		if (pid == 0) {
			close(pipe_fd[0]);
			dup2(pipe_fd[1], STDOUT_FILENO);
			execl("/bin/sh", "sh", "-c", command, NULL);
			free(file);
			return NULL;
		}
		if (pid == -1) {
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			so_fclose(file);
			return NULL;
		}

		file->pid = pid;

		close(pipe_fd[1]);
		file->fd = pipe_fd[0];

		return file;
	}

	if (strcmp(type, "w") == 0) {
		file->permision[0] = 'w';
		file->permision[1] = '\0';
		pid_t pid = fork();

		if (pid == 0) {
			close(pipe_fd[1]);
			dup2(pipe_fd[0], STDIN_FILENO);
			execl("/bin/sh", "sh", "-c", command, NULL);
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			so_fclose(file);
			return NULL;
		}
		if (pid == -1) {
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			so_fclose(file);
			return NULL;
		}
			file->pid = pid;
		file->fd = pipe_fd[1];
		close(pipe_fd[0]);
		return file;
	}
	so_fclose(file);
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	int stat;

	so_fflush(stream);
	close(stream->fd);
	int sstt = waitpid(stream->pid, &stat, 0);

	so_fclose(stream);
	if (WIFEXITED(stat) < 0 || sstt < 0)
		return SO_EOF;
	return WEXITSTATUS(stat);
}
