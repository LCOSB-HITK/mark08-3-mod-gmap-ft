import os

author = "Lucius Pertis"
project_url = "https://github.com/LCOSB-HITK/"

attribution_msg = f"/***\n\tauthor: {author}\n\tComplete project details at {project_url}\n***/\n\n"

def create_library_files(library_name):
    # Create .h file in the include folder
    header_file_path = os.path.join("include", f"{library_name}.h")
    with open(header_file_path, "w") as header_file:
        header_file.write(attribution_msg)
        header_file.write("#ifndef " + library_name.upper() + "_H\n")
        header_file.write("#define " + library_name.upper() + "_H\n\n")

        # Include default libraries
        include_default_libraries(header_file)

        header_file.write("\n\n#endif // " + library_name.upper() + "_H\n")

    # Create .c file in the current directory
    source_file_path = f"{library_name}.c"
    with open(source_file_path, "w") as source_file:
        source_file.write(attribution_msg)
        source_file.write(f'\n#include "{library_name}.h"\n\n')

        # # Include default libraries
        # include_default_libraries(source_file)

        # Ask if additional libraries should be included
        include_additional_libraries(source_file)

def include_default_libraries(file):
    def_libs = []
    
    if not os.path.exists("default_includes.txt"): return
    with open("default_includes.txt", "r") as default_includes_file:
        if default_includes_file is None: return
        def_libs = default_includes_file.read()

    if def_libs == None: return

    def_libs = def_libs.split("\n")
    while def_libs.count(""): def_libs.remove("")

    include_directives = [f'#include "{lib}.h"\n' for lib in def_libs]

    # Ask user if default libraries should be included
    print(">>> default libraries:")
    for lib in def_libs : print(">> >> ", lib) 

    include_defaults = input(">>> Include def libs? (y/n): ").lower() == 'y'
    if include_defaults:
        file.writelines(include_directives)

def include_additional_libraries(file):
    # Ask user for additional libraries to include
    include_additional = input(">> Include additional lib? (y/n): ").lower() == 'y'

    while include_additional:
        library_name = input(">> library name (without .h): ")
        include_directive = f'#include "{library_name}.h"\n'
        file.write(include_directive)

        # # Store additional library in default_includes.txt
        # with open("default_includes.txt", "a") as default_includes_file:
        #     default_includes_file.write(f'{library_name}\n')

        include_additional = input(">> Include another additional library? (y/n): ").lower() == 'y'

if __name__ == "__main__":
    # Ask for the library name and current directory
    current_directory = input(">> root dir path (parent of include folder): ")

    # Change to the specified directory
    os.chdir(current_directory)

    while(True):
        library_name = input("\n>> C library name (without .h extension): ")

        # Create library files and include default libraries
        create_library_files(library_name)

        print(f"\n>> Library files {library_name}.h and {library_name}.c created successfully.\n")

        if input(">> Create more libs? (y/n):").lower() != 'y': break


