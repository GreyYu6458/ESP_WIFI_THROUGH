import os

basic_command = "idf_component_register"

suffix = ".c"

if __name__ == "__main__":
    # backup
    with open("CMakeLists.txt", "r") as p:
        with open("CMakeLists_b.txt","w+") as pb:
            pb.write(p.read())
    file_list = []
    dir_list = []
    # search files
    for root, _, files in os.walk("."):
            dir_list.append(root)
            for f in files:
                if f[-2:] == suffix:
                    file_list.append(root +"/"+ f)

    with open("CMakeLists.txt", "w+") as p:
        p.write(basic_command+"(")
        p.write("SRCS \r\n")
        for f in file_list:
            p.write('"'+f+'"' + "\r\n")
        p.write("INCLUDE_DIRS \r\n")
        for d in dir_list:
            p.write('"'+d+'"' + "\r\n")
        p.write(")")
        