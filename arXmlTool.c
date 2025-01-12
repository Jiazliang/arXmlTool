#define LIBXML_STATIC  // Use static library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#ifdef _WIN32
#include <io.h>     // For _access function in Windows
#else
#include <unistd.h> // For access function in Unix systems
#endif

#define MAX_FILES 1024
#define MAX_PATH 256
#define MAX_CMD_LENGTH 4096
#define MAX_BUFFER_SIZE 8192

typedef enum {
    MODE_MERGE,
    MODE_COMPARE,
    MODE_GENERATE,
    MODE_UNKNOWN
} OperationMode;

typedef struct {
    OperationMode mode;
    char input_files[MAX_FILES][MAX_PATH];
    int input_file_count;
    char output_file[MAX_PATH];
    char output_dir[MAX_PATH];
} ProgramOptions;

static xmlChar* get_short_name(xmlNodePtr node);
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc);

OperationMode parse_mode(const char* mode_str) {
    if (strcmp(mode_str, "merge") == 0) return MODE_MERGE;
    if (strcmp(mode_str, "compare") == 0) return MODE_COMPARE;
    if (strcmp(mode_str, "generate") == 0) return MODE_GENERATE;
    return MODE_UNKNOWN;
}

void print_usage() {
    printf("Usage:\n");
    printf("  arXmlTool.exe <mode> [options]\n");
    printf("  arXmlTool.exe -f <command_file>\n\n");
    printf("Modes:\n");
    printf("  merge    - Merge multiple ARXML files\n");
    printf("  compare  - Compare ARXML files\n");
    printf("  generate - Generate ARXML file\n\n");
    printf("Merge mode options:\n");
    printf("  -a <file.arxml>  Specify input file (can be used multiple times)\n");
    printf("  -m <file.arxml>  Specify output file\n");
    printf("  -o <directory>   Specify output directory\n");
}

int parse_merge_options(int argc, char *argv[], ProgramOptions *opts) {
    int opt;
    opts->input_file_count = 0;
    memset(opts->output_file, 0, MAX_PATH);  // Clear output filename
    
    // Set default output directory to current directory
    strncpy(opts->output_dir, ".", MAX_PATH - 1);
    
    // Reset getopt
    optind = 0;
    
    while ((opt = getopt(argc, argv, "a:m:o:")) != -1) {
        switch (opt) {
            case 'a':
                if (opts->input_file_count >= MAX_FILES) {
                    printf("Error: Number of input files exceeds limit (%d)\n", MAX_FILES);
                    return 0;
                }
                strncpy(opts->input_files[opts->input_file_count], optarg, MAX_PATH - 1);
                opts->input_file_count++;
                break;
            case 'm':
                strncpy(opts->output_file, optarg, MAX_PATH - 1);
                break;
            case 'o':
                strncpy(opts->output_dir, optarg, MAX_PATH - 1);
                break;
            case '?':
                printf("Error: Invalid option or missing argument\n");
                return 0;
            default:
                printf("Error: Unknown option %c\n", opt);
                return 0;
        }
    }
    
    // Check required parameters: at least one input file and one output file
    if (opts->input_file_count == 0 || strlen(opts->output_file) == 0) {
        printf("Error: Merge mode requires at least one input file (-a) and one output file (-m)\n");
        printf("Debug: input_file_count = %d, output_file = '%s'\n", 
               opts->input_file_count, opts->output_file);
        return 0;
    }
    
    // Check if files exist
    for (int i = 0; i < opts->input_file_count; i++) {
        FILE* file = fopen(opts->input_files[i], "r");
        if (!file) {
            printf("Error: Cannot open input file '%s'\n", opts->input_files[i]);
            return 0;
        }
        fclose(file);
    }
    
    // Check if output directory exists
    if (strcmp(opts->output_dir, ".") != 0) {
        #ifdef _WIN32
        if (_access(opts->output_dir, 0) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #else
        if (access(opts->output_dir, F_OK) != 0) {
            printf("Error: Output directory '%s' does not exist\n", opts->output_dir);
            return 0;
        }
        #endif
    }
    
    return 1;
}

// New: Read command line from file
char** read_command_from_file(const char* filename, int* argc) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open command file '%s'\n", filename);
        return NULL;
    }

    char line[MAX_CMD_LENGTH];
    if (!fgets(line, sizeof(line), file)) {
        printf("Error: Command file is empty\n");
        fclose(file);
        return NULL;
    }
    fclose(file);

    // Remove trailing newline
    line[strcspn(line, "\r\n")] = 0;

    // Count arguments
    int count = 1; // Program name takes first position
    char* p = line;
    while (*p) {
        if (*p == ' ' || *p == '\t') {
            count++;
            while (*p && (*p == ' ' || *p == '\t')) p++;
        } else {
            p++;
        }
    }

    // Allocate memory
    char** argv = (char**)malloc((count + 1) * sizeof(char*));
    if (!argv) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    // First argument is program name
    argv[0] = strdup("arXmlTool.exe");

    // Split command line
    int i = 1;
    char* token = strtok(line, " \t");
    while (token) {
        argv[i] = strdup(token);
        token = strtok(NULL, " \t");
        i++;
    }
    argv[i] = NULL;
    *argc = i;

    return argv;
}

// New: Free command line arguments memory
void free_command_args(char** argv) {
    if (!argv) return;
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

// Compare node names (ignore namespaces)
static int compare_node_names(const xmlChar* name1, const xmlChar* name2) {
    // Skip namespace prefix
    const xmlChar* local_name1 = xmlStrchr(name1, ':');
    const xmlChar* local_name2 = xmlStrchr(name2, ':');
    
    // If no namespace prefix, use full name
    if (local_name1 == NULL) local_name1 = name1;
    else local_name1++; // Skip colon
    if (local_name2 == NULL) local_name2 = name2;
    else local_name2++; // Skip colon
    
    return xmlStrcmp(local_name1, local_name2);
}

// Merge function implementation
int merge_arxml_files(const ProgramOptions *opts) {
    xmlDocPtr base_doc = NULL;
    xmlNodePtr root_node = NULL;
    
    // Parse base file
    base_doc = xmlReadFile(opts->input_files[0], NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOBLANKS | XML_PARSE_COMPACT);
    if (base_doc == NULL) {
        printf("Error: Cannot parse base file '%s'\n", opts->input_files[0]);
        return 0;
    }
    
    // Get root node
    root_node = xmlDocGetRootElement(base_doc);
    if (root_node == NULL) {
        printf("Error: File '%s' is empty\n", opts->input_files[0]);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    // Process other files
    for (int i = 1; i < opts->input_file_count; i++) {
        xmlDocPtr doc = xmlReadFile(opts->input_files[i], NULL, XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            printf("Error: Cannot parse file '%s'\n", opts->input_files[i]);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        xmlNodePtr cur_root = xmlDocGetRootElement(doc);
        if (cur_root == NULL) {
            printf("Error: File '%s' is empty\n", opts->input_files[i]);
            xmlFreeDoc(doc);
            xmlFreeDoc(base_doc);
            return 0;
        }
        
        // Recursively merge nodes
        xmlNodePtr cur = cur_root->children;
        while (cur != NULL) {
            merge_node(root_node, cur, base_doc);
            cur = cur->next;
        }
        
        xmlFreeDoc(doc);
    }
    
    // Save merged document
    xmlKeepBlanksDefault(0);
    if (xmlSaveFormatFileEnc(opts->output_file, base_doc, "UTF-8", 2) < 0) {
        printf("Error: Cannot save output file '%s'\n", opts->output_file);
        xmlFreeDoc(base_doc);
        return 0;
    }
    
    xmlFreeDoc(base_doc);
    // Only print completion message if all files were successfully merged
    if (opts->input_file_count > 1) {
        printf("Merge completed, output file: %s\n", opts->output_file);
    }
    return 1;
}

// Check number of SHORT-NAME nodes in node and all its children
static int count_short_names(xmlNodePtr node) {
    // If not an element node, return 0
    if (node->type != XML_ELEMENT_NODE) {
        return 0;
    }
    
    int count = 0;
    xmlNodePtr cur = node->children;
    
    // Recursively check all child nodes
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            count++;
            if (count > 1) {
                return count;  // If multiple found, return immediately
            }
        } else if (cur->type == XML_ELEMENT_NODE) {
            count += count_short_names(cur);
            if (count > 1) {
                return count;  // If multiple found, return immediately
            }
        }
        cur = cur->next;
    }
    
    return count;
}

// Recursively merge nodes
static void merge_node(xmlNodePtr base_parent, xmlNodePtr input_node, xmlDocPtr doc) {
    // Skip text nodes and comment nodes
    if (input_node->type != XML_ELEMENT_NODE) {
        return;
    }

    xmlChar* input_name = get_short_name(input_node);
    xmlNodePtr existing_node = NULL;
    
    // Search for nodes of the same type in base_parent's children
    xmlNodePtr cur = base_parent->children;
    while (cur != NULL) {
        // Compare only nodes of the same type, ignore namespaces
        if (compare_node_names(cur->name, input_node->name) == 0) {
            xmlChar* cur_name = get_short_name(cur);
            
            // If both nodes have SHORT-NAME, compare names
            if (input_name != NULL && cur_name != NULL) {
                if (!xmlStrcmp(cur_name, input_name)) {
                    existing_node = cur;
                }
            } 
            // If neither node has SHORT-NAME, consider them matching
            else if (input_name == NULL && cur_name == NULL) {
                existing_node = cur;
            }
            
            if (cur_name != NULL) {
                xmlFree(cur_name);
            }
            
            if (existing_node) {
                break;
            }
        }
        cur = cur->next;
    }
    
    if (input_name != NULL) {
        xmlFree(input_name);
    }
    
    if (existing_node != NULL) {
        // Found matching node
        if (input_name != NULL && count_short_names(input_node) == 1) {
            // If it's a minimal unit with SHORT-NAME, discard entire subtree
            return;
        }
        // Otherwise process child nodes recursively
        xmlNodePtr input_child = input_node->children;
        while (input_child != NULL) {
            merge_node(existing_node, input_child, doc);
            input_child = input_child->next;
        }
        return;
    }
    
    // No matching node found, copy entire subtree
    xmlNodePtr copy = xmlNewNode(NULL, input_node->name);
    
    xmlNodePtr child = input_node->children;
    while (child != NULL) {
        if (child->type == XML_ELEMENT_NODE) {
            // 递归复制元素节点
            merge_node(copy, child, doc);
        } else if (child->type == XML_TEXT_NODE) {
            xmlChar* content = xmlNodeGetContent(child);
            xmlNodePtr text = xmlNewText(content);
            xmlAddChild(copy, text);
            xmlFree(content);
        }
        child = child->next;
    }
    xmlAddChild(base_parent, copy);
}

// Implement helper function
static xmlChar* get_short_name(xmlNodePtr node) {
    xmlNodePtr cur = node->children;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SHORT-NAME"))) {
            return xmlNodeGetContent(cur);
        }
        cur = cur->next;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Initialize libxml2
    LIBXML_TEST_VERSION
    
    char** cmd_argv = NULL;
    int cmd_argc = 0;
    int using_cmd_file = 0;

    // Check if using command file
    if (argc == 3 && strcmp(argv[1], "-f") == 0) {
        cmd_argv = read_command_from_file(argv[2], &cmd_argc);
        if (!cmd_argv) {
            return 1;
        }
        argv = cmd_argv;
        argc = cmd_argc;
        using_cmd_file = 1;
    }

    if (argc < 2) {
        print_usage();
        if (using_cmd_file) free_command_args(cmd_argv);
        return 1;
    }

    ProgramOptions opts = {0};
    opts.mode = parse_mode(argv[1]);

    if (opts.mode == MODE_UNKNOWN) {
        printf("Error: Unknown operation mode '%s'\n", argv[1]);
        print_usage();
        if (using_cmd_file) free_command_args(cmd_argv);
        return 1;
    }

    int success = 0;
    switch (opts.mode) {
        case MODE_MERGE: {
            // Create new argument array, keep program name
            char** new_argv = (char**)malloc((argc - 1) * sizeof(char*));
            if (!new_argv) {
                printf("Error: Memory allocation failed\n");
                if (using_cmd_file) free_command_args(cmd_argv);
                return 1;
            }
            new_argv[0] = argv[0];  // Keep program name
            for (int i = 2; i < argc; i++) {
                new_argv[i-1] = argv[i];
            }
            success = parse_merge_options(argc - 1, new_argv, &opts);
            free(new_argv);
            
            if (success) {
                printf("Merge mode:\n");
                for (int i = 0; i < opts.input_file_count; i++) {
                    printf("Input file %d: %s\n", i + 1, opts.input_files[i]);
                }
                
                // Execute merge operation
                success = merge_arxml_files(&opts);
            }
            break;
        }
            
        case MODE_COMPARE:
            printf("Compare mode not implemented\n");
            break;
            
        case MODE_GENERATE:
            printf("Generate mode not implemented\n");
            break;
            
        default:
            success = 0;
            break;
    }

    if (!success) {
        print_usage();
        if (using_cmd_file) free_command_args(cmd_argv);
        return 1;
    }

    if (using_cmd_file) free_command_args(cmd_argv);
    
    // Cleanup libxml2
    xmlCleanupParser();
    
    return 0;
} 