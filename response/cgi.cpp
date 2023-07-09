#include <sys/wait.h>
#include "../webserv.hpp"
#include "../config.hpp"
#include <fcntl.h>
#include <cstring>
#include <cstdio>

char** get_env(HttpResponse& response)
{
    std::vector<std::string> vect;
    (void) response;
    vect.push_back("REQUEST_METHOD=" + response.request.method);
    vect.push_back("SCRIPT_FILENAME=" + response.path_file);
    vect.push_back("PATH_INFO=" + response.path_file);
    vect.push_back("CONTENT_LENGTH=0"  + response.headers["Content-Length"]);
    vect.push_back("REDIRECT_STATUS=200");
    vect.push_back("GATEWAY_INTERFACE=CGI/1.1");
    vect.push_back("QUERY_STRING=" + response.query_str);
    // vect.push_back("REQUEST_url=/Users/kadjane/Desktop/web_serv2/test.php"); 
    // vect.push_back("CONTENT_TYPE=text/html");


    // vect.push_back("REQUEST_url=/Users/kadjane/Desktop/web_serv2/test.php"); 
    // vect.push_back("CONTENT_TYPE=text/html");
    // vect.push_back("SERVER_PORT=8080");  
    // vect.push_back("HTTP_HOST=0.0.0.0"); 
    // vect.push_back("SERVER_PROTOCOL=HTTP/1.1");
    // vect.push_back("SCRIPT_NAME=/Users/kadjane/Desktop/web_serv2/test.php");

    char** env = new char*[vect.size() + 1];
    size_t i;
    for (i = 0; i < vect.size(); ++i)
    {
        env[i] = new char[vect[i].length() + 1];
        strcpy(env[i], vect[i].c_str());
    }
    env[vect.size()] = NULL; 
    return env;
}

// void parse_query_string(HttpResponse &response)
// {
// 	std::string query_str_parsed;
// 	std::vector<std::string> space;
// 	space.push_back("%20");
// 	space.push_back("%2B");
// 	space.push_back("+");
// 	int find = true;

//     query_str_parsed += response.query_str;
// 	while (find)
// 	{
// 		find = false;
// 		for (std::vector<std::string>::iterator space_it = space.begin(); space_it != space.end(); space_it++)
// 		{
// 			if (query_str_parsed.find(*space_it) != std::string::npos)
// 			{
// 				query_str_parsed = query_str_parsed.substr(0,query_str_parsed.find(*space_it)) + " "
//                     + query_str_parsed.substr(query_str_parsed.find(*space_it) + (*space_it).length(), query_str_parsed.length());
//                 std::cout << "\033[33m" << "query_parsed == {" << query_str_parsed << "}" << "\033[00m" << std::endl;
// 				find = true;
//                 break ;
// 			}
// 		}
// 	}
// 	std::cout << "\033[32m" << "*******> {" << query_str_parsed << "\033[0m" << std::endl;
// }

void cgi_response_content(HttpResponse & response)
{
    std::ifstream out_file("output.txt", std::ifstream::binary);
    std::ofstream file("out");
    std::string line;

    while(std::getline(out_file, line) && line != "\r")
    {
        if (line.find("Content-type") != std::string::npos)
        {
            int length = line.find(";") - (line.find(" ") + 1);
            response.headers["Content-Type"] = line.substr(line.find(" ") + 1, length);
        }
    }
    while (std::getline(out_file, line))
    {
        line += "\n";
        file << line;
    }
    response.path_file = "out";
}

void    execute_cgi(HttpResponse &response)
{
    pid_t pid = fork();
    CGI cgi;
    int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    std::string path = response.path_file;
    std::string path_executable = response.location_it->cgi[0].cgi_pass;

    if (pid == 0)
    {
        char **env;
        env = get_env(response);
        char* const argv[] = {(char*)path_executable.c_str(), (char*)path.c_str(), NULL};
        if (dup2(output_fd, 1) == -1 || dup2(output_fd, 2) == -1) {
            std::cout << "Error duplicating file descriptor." << std::endl;
            return ;
        }
        if (execve(argv[0], argv, env) == -1){
            std::cout << "Error executing CGI script." << std::endl;
            return ;
        }

    }
    else
    {
        waitpid(pid, NULL, 0);
        cgi_response_content(response);
        close(output_fd);
    }
}
