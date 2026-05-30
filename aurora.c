#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_STR 512

char *trim(char *str) {
	while (isspace((unsigned char)*str)) str++;
	if (*str == '\0') return str;
	char *end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;
	if ((*str == '"' && *end == '"') || (*str == '\'' && *end == '\'')) { str++; end--; }
	*(end + 1) = '\0';
	return str;
}

int main(int argc, char *argv[]) {
	if (argc < 2) return fprintf(stderr, "Usage: %s <test-suite-name>\n", argv[0]), 1;
	char target_section[MAX_STR]; snprintf(target_section, sizeof(target_section), "[%s]", argv[1]);
	char exec_path[MAX_STR] = {0}, test_folder[MAX_STR] = {0}, line[MAX_STR * 2];
	FILE *cfg = fopen("aurora_config.toml", "r");
	if (!cfg) return fprintf(stderr, "Error: Missing 'aurora_config.toml'\n"), 1;
	bool in_target_section = false;
	while (fgets(line, sizeof(line), cfg)) {
		char *comment = strchr(line, '#'); if (comment) *comment = '\0';
		char *trimmed = trim(line); if (*trimmed == '\0') continue;
		if (*trimmed == '[') { in_target_section = (strcmp(trimmed, target_section) == 0); continue; }
		if (!in_target_section) continue;
		char *eq = strchr(line, '='); if (!eq) continue;
		*eq = '\0'; char *key = trim(line), *val = trim(eq + 1);
		if (strcmp(key, "exec") == 0) strncpy(exec_path, val, MAX_STR - 1);
		if (strcmp(key, "test_folder") == 0) strncpy(test_folder, val, MAX_STR - 1);
	}
	fclose(cfg);
	if (!exec_path[0] || !test_folder[0]) 
		return fprintf(stderr, "Error: Suite '%s' missing or incomplete in config.\n", argv[1]), 1;
	char suite_path[MAX_STR * 2]; snprintf(suite_path, sizeof(suite_path), "%s/tests.toml", test_folder);
	FILE *stf = fopen(suite_path, "r");
	if (!stf) return fprintf(stderr, "Error: Missing '%s'\n", suite_path), 1;
	char name[MAX_STR] = {0}, file[MAX_STR] = {0}, expect[MAX_STR] = {0};
	int expected_return = 0, failed_count = 0; bool skip = false;
	while (fgets(line, sizeof(line), stf)) {
		char *comment = strchr(line, '#'); if (comment) *comment = '\0';
		char *trimmed = line; while (isspace((unsigned char)*trimmed)) trimmed++;
		if (*trimmed == '\0') continue;
		if (*trimmed == '[') {
			if (name[0] && !skip) {
				char cmd[MAX_STR * 3], output[8192] = {0}, buffer[512];
				snprintf(cmd, sizeof(cmd), "%s %s/%s 2>&1", exec_path, test_folder, file);
				FILE *pipe = popen(cmd, "r");
				if (pipe) {
					while (fgets(buffer, sizeof(buffer), pipe)) strncat(output, buffer, sizeof(output) - strlen(output) - 1);
					int raw = pclose(pipe), ret = (raw == -1) ? -1 : (raw & 0xff00) >> 8;
					size_t out_len = strlen(output);
					while (out_len > 0 && (output[out_len - 1] == '\n' || output[out_len - 1] == '\r')) output[--out_len] = '\0';
					bool out_match = (strcmp(output, expect) == 0), code_match = (ret == expected_return);
					if (!out_match || !code_match) {
						if (failed_count == 0) printf("Failure\nThese tests failed\n");
						failed_count++; printf("- Test \"%s\":\n", name);
						if (!code_match) printf("\t- Expected return code %d but got %d\n", expected_return, ret);
						if (!out_match) printf("\t- Expected output \"%s\" but got \"%s\"\n", expect, output);
					}
				}
			}
			char *close = strchr(trimmed, ']'); if (close) *close = '\0';
			strncpy(name, trim(trimmed + 1), MAX_STR - 1);
			skip = false; expected_return = 0; file[0] = '\0'; expect[0] = '\0'; continue;
		}
		char *eq = strchr(line, '='); if (!eq) continue;
		*eq = '\0'; char *key = trim(line), *val = trim(eq + 1);
		if (strcmp(key, "file") == 0) strncpy(file, val, MAX_STR - 1);
		else if (strcmp(key, "expect") == 0) strncpy(expect, val, MAX_STR - 1);
		else if (strcmp(key, "return") == 0) expected_return = atoi(val);
		else if (strcmp(key, "skip") == 0) skip = (strcmp(val, "true") == 0);
	}
	fclose(stf);
	if (name[0] && !skip) {
		char cmd[MAX_STR * 3], output[8192] = {0}, buffer[512];
		snprintf(cmd, sizeof(cmd), "%s %s/%s 2>&1", exec_path, test_folder, file);
		FILE *pipe = popen(cmd, "r");
		if (pipe) {
			while (fgets(buffer, sizeof(buffer), pipe)) strncat(output, buffer, sizeof(output) - strlen(output) - 1);
			int raw = pclose(pipe), ret = (raw == -1) ? -1 : (raw & 0xff00) >> 8;
			size_t out_len = strlen(output);
			while (out_len > 0 && (output[out_len - 1] == '\n' || output[out_len - 1] == '\r')) output[--out_len] = '\0';
			bool out_match = (strcmp(output, expect) == 0), code_match = (ret == expected_return);
			if (!out_match || !code_match) {
				if (failed_count == 0) printf("Failure\nThese tests failed\n");
				failed_count++; printf("- Test \"%s\":\n", name);
				if (!code_match) printf("\t- Expected return code %d but got %d\n", expected_return, ret);
				if (!out_match) printf("\t- Expected output \"%s\" but got \"%s\"\n", expect, output);
			}
		}
	}
	if (failed_count == 0) printf("Congratulations all tests passed! :)\n");
	return failed_count > 0 ? 1 : 0;
}