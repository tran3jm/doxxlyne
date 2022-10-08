#include <stdio.h>
#include <string.h>
void manip_nl_character(char* input, char* returnBuffer) {
    int len = strlen(input);
    char out_line[256];
    memset(out_line, 0, sizeof(out_line));
    char backslash = '\\';
    int i       = 0;
    int index   = 0;
    int n_slashes = 0;
    while(i <= len){
        if(input[i] == backslash) {
            n_slashes++;
            i++;
            char next_char = input[i];
            if(next_char == 'n') {
                out_line[index] = '\n';
            } else if(next_char == 't') {
                out_line[index] = '\t';
            } else {
                //Error_throw_printf("Invalid escaped character %c", input[i]);
            }
        } else {
            out_line[index] = input[i];
        }
        index++;
        i++;
    }
    snprintf(returnBuffer, strlen(out_line) + n_slashes, "%s", out_line);
}

int main(int argc, char** argv) {
    char* example = "ABC\\nD";
    printf("First Print: %s\n", example);
    char out_line[256];
    manip_nl_character(example, out_line);
    printf("Final Print: %s\n", out_line);

    char* example2 = "AABB\\tCC\\nDD";
    printf("First Print: %s\n", example2);
    manip_nl_character(example2, out_line);
    printf("Final Print: %s\n", out_line);
}
