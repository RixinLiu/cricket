// The MIT License (MIT)
// 
// Copyright (c) 2012 Niklas E.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#define _GNU_SOURCE

#include "log.h"

#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

static struct timeval start_time = {0};
static FILE* warning_log_file = NULL;

static const char* to_string_plain(log_level level)
{
	static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG"};
	if(level > LOG_DEBUG){
		return buffer[LOG_DEBUG];
	}
	return buffer[(int)level];
}

static void init_warning_log_file(void)
{
	if (warning_log_file != NULL) {
		return;
	}

	Dl_info info;
	if (dladdr((void*)init_log, &info) == 0 || info.dli_fname == NULL) {
		return;
	}

	char binary_path[PATH_MAX];
	char component_dir[PATH_MAX];
	char project_dir[PATH_MAX];
	char warning_path[PATH_MAX];
	const char* warning_filename = "/cricket_warning.log";
	size_t project_dir_len;
	size_t warning_filename_len;

	snprintf(binary_path, sizeof(binary_path), "%s", info.dli_fname);
	snprintf(component_dir, sizeof(component_dir), "%s", dirname(binary_path));
	snprintf(project_dir, sizeof(project_dir), "%s", dirname(component_dir));
	project_dir_len = strnlen(project_dir, sizeof(project_dir));
	warning_filename_len = strlen(warning_filename);
	if (project_dir_len + warning_filename_len + 1 > sizeof(warning_path)) {
		return;
	}

	memcpy(warning_path, project_dir, project_dir_len);
	memcpy(warning_path + project_dir_len, warning_filename, warning_filename_len + 1);

	warning_log_file = fopen(warning_path, "a");
}

static FILE* get_log_stream(log_level level)
{
	if (level == LOG_WARNING) {
		return warning_log_file != NULL ? warning_log_file : stderr;
	}
	return stdout;
}

struct log_data* get_log_data() {
	static struct log_data log_data;
	return &log_data;
}

int str_find_last_of(const char* to_search, char to_find) {
	int len = strlen(to_search);
	for(int i = len-1; i >= 0; --i) {
		if(to_search[i] == to_find)
			return i;
	}
	return 0;
}

void str_strip(char* to_strip, int offset) {
	int len = strlen(to_strip);
	for(int i = offset; i < len; ++i) {
		to_strip[i-offset] = to_strip[i];
	}
	to_strip[len-offset] = '\0';
}

void init_log(char log_level, const char* proj_root)
{
	get_log_data()->curr_level=log_level;
	get_log_data()->project_offset = str_find_last_of(proj_root, '/');
	gettimeofday(&start_time, 0);
	init_warning_log_file();
}

void now_time(char* buf)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%x %X", localtime(&tv.tv_sec));
	sprintf(buf, "%s.%06ld", buffer, (long)tv.tv_usec);
}

void delta_time(char* buf)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	timersub(&tv, &start_time, &tv);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%X", localtime(&tv.tv_sec));
	sprintf(buf, "+%s.%06ld", buffer, (long)tv.tv_usec);
}

const char* to_string(log_level level)
{
#ifdef NOCOLORS
	static const char* const buffer[] = {"ERROR", "WARNING", "INFO", "DEBUG"};
#else
	static const char* const buffer[] = {"\033[1m\033[31mERROR\033[0m", "\033[33mWARNING\033[0m", "\033[34mINFO\033[0m", "\033[32mDEBUG\033[0m"};
#endif //NOCOLORS
	if(level > LOG_DEBUG){
		return buffer[LOG_DEBUG];
	}
	else return buffer[(int)level];
}

void loggf(log_level level, const char* formatstr, ... )
{
	va_list vararg;
	va_start(vararg, formatstr);

	FILE* stream = get_log_stream(level);
	char time[64];
#ifdef DELTA_TIME
	delta_time(time);
#else
	now_time(time);
#endif //DELTA_TIME
	fprintf(stream, "%s %s:\t", time,
		level == LOG_WARNING ? to_string_plain(level) : to_string(level));
	vfprintf(stream, formatstr, vararg);
	fprintf(stream, "\n");
	fflush(stream);
	va_end(vararg);
}

void loggfe(log_level level, int line, const char* file, const char* formatstr, ... )
{
	va_list vararg;
	va_start(vararg, formatstr);

	FILE* stream = get_log_stream(level);
	char time[64];
#ifdef DELTA_TIME
	delta_time(time);
#else
	now_time(time);
#endif //DELTA_TIME
	fprintf(stream, "%s %7s: ", time,
		level == LOG_WARNING ? to_string_plain(level) : to_string(level));
	vfprintf(stream, formatstr, vararg);
	char stripped[64];
	strcpy(stripped, file);
	str_strip(stripped, get_log_data()->project_offset);
#ifdef NOCOLORS
	fprintf(stream, "\tin %s:%d\n", stripped, line);
#else
	if (level == LOG_WARNING) {
		fprintf(stream, "\tin %s:%d\n", stripped, line);
	} else {
		fprintf(stream, "\tin \033[4m%s:%d\033[0m\n", stripped, line);
	}
#endif //NOCOLORS
	fflush(stream);
	va_end(vararg);
}
