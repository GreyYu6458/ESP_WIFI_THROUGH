import os

basic_command = "idf_component_register"
include_command = "include_directories"

suffix = ".c"

if __name__ == "__main__":
    # backup
    with open("CMakeLists.txt", "r") as p:
        with open("CMakeLists_b.txt","w+") as pb:
            pb.write(p.read())

    with open("main/CMakeLists.txt", "r") as p:
        with open("main/CMakeLists_b.txt","w+") as pb:
            pb.write(p.read())

    file_list = []
    dir_list = []
    # search files
    for root, _, files in os.walk("main"):
            dir_list.append(root)
            for f in files:
                if f[-2:] == suffix:
                    file_list.append(root +"/"+ f)

    with open("main/CMakeLists.txt", "w+") as p:
        p.write(basic_command+"(")
        p.write("SRCS \n")
        for f in file_list:
            p.write("\t"+'".'+f[4:]+'"' + "\n")
        
        p.write("INCLUDE_DIRS \n")
        for d in dir_list:
            p.write('\t".'+d[4:] + '"' + '\n')
        p.write(")")
    
    with open("CMakeLists.txt", "w+") as pc:
        pc.write('cmake_minimum_required(VERSION 3.5)\n')
        pc.write('include($ENV{IDF_PATH}/tools/cmake/project.cmake)\n')

        pc.write(include_command+"(\n")
        for d in dir_list:
            pc.write("\t"+d+"\n")
        pc.write(")\n")

        pc.write('project(UART_UDP_BRIDGE)')
        
        