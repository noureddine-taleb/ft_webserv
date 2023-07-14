#include <sys/wait.h>
#include "../webserv.hpp"
#include "../config.hpp"
#include <fcntl.h>
#include <cstring>
#include <cstdio>

char** get_env(HttpResponse& response)
{
    std::vector<std::string> vect;
    std::string content_length = ft_tostring(response.request.content.size());
    std::string str(response.request.content.begin(), response.request.content.end());
    vect.push_back("REQUEST_METHOD=" + response.request.method);
    vect.push_back("SCRIPT_FILENAME=" + response.path_file);
    vect.push_back("PATH_INFO=" + response.path_file);
    vect.push_back("CONTENT_LENGTH="  + content_length);
    vect.push_back("REDIRECT_STATUS=200");
    vect.push_back("GATEWAY_INTERFACE=CGI/1.1");
    vect.push_back("QUERY_STRING=" + response.query_str);
    vect.push_back("CONTENT_TYPE=" + response.request.headers["Content-Type"]);
    vect.push_back("HTTP_COOKIE=" + response.request.headers["Cookie"]);

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

void cgi_response_content(HttpResponse & response, std::string &name_output)
{
    static int i;
    if (name_output != "output")
    {
        if (response.cgi_it->file_extension == "php")
        {
            std::ifstream out_file(name_output, std::ifstream::binary);
            std::string out = "out";
            out = generate_filename(out, &i);
            std::ofstream file(out);
            response.file_name_genarated.push_back(out);
            std::string line;
            while(std::getline(out_file, line) && line != "\r")
            {
                int length = line.find(";") - (line.find(" ") + 1);
                std::string key = line.substr(0,line.find(" ")-1);
                std::string value = line.substr(line.find(" ")+1, length);
                // if (line.find("Set-Cookie") != std::string::npos)
                // {
                //     key = "Set-cookie";
                // }
                response.headers[key] = value;

                //                 if (line.find("Set-Cookie") != std::string::npos)
                // response.headers["Set-cookie"] = line.substr(line.find(" ") + 1, length);
                // else
                //     response.headers["Content-Type"] = "text/html";
            }
            //     if (line.find("Cookie") != std::string::npos && response.request.headers["Cookie"].empty())
            //     {
            //         int length = line.find(";") - (line.find(" ") + 1);
            //         response.headers["Set-Cookie"] = line.substr(line.find(" ") + 1, length);
            //     }
            // response.headers["Cache-Control"] = "no-store, no-cache, must-revalidate";
            // response.headers["Pragma"] = "no-cache";
            // response.headers["X-Powered-By"] = "PHP/8.2.1";
            // response.headers["Expires"] = "Thu, 19 Nov 1981 08:52:00 GMT";
            while (std::getline(out_file, line))
            {
                line += "\n";
                file << line;
            }
            response.path_file = out;
            // response.request.headers["cookies"] = response.cookies;
            // std::cout << YELLOW<<"{"<< response.cookies<<"}" << END<< std::endl;
            // std::cout <<SKY <<  response.path_file << END << std::endl;
            file.close();
            out_file.close();
        }
        else
        {
            response.headers["Content-type"] = "text/html";
            response.path_file = name_output;
        }
    }

}

int    execute_cgi(HttpResponse &response)
{
     int output_fd = 0;

    std::string name_output = "output";
    if (response.pid == -1)
    {
        response.pid = fork();
        static int i;
        name_output = generate_filename(name_output, &i);
        output_fd = open(name_output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        response.file_name_genarated.push_back(name_output);///////////////////
            char **env;
            env = get_env(response);
        if (response.pid == 0)
        {
            std::string path = response.path_file;
            std::string path_executable = response.cgi_it->cgi_pass;
            int input_fd  = 0;
            if (response.request.method == "POST")
                input_fd = open("cgi.txt", O_RDONLY , 0666);
            char* const argv[] = {(char*)path_executable.c_str(), (char*)path.c_str(), NULL};
            if (dup2(output_fd, 1) == -1 || dup2(output_fd, 2) == -1) 
            {
                std::cerr << "Error duplicating file descriptor." << std::endl;
                ft_send_error(500, response);
                close(output_fd);
                return(1);
            }
            close(output_fd);
            if (input_fd > 0 && dup2(input_fd, STDIN_FILENO) < 0) 
            {
                // std::cerr << "Error duplicating input file descriptor." << std::endl;
                ft_send_error(500, response);
                close(input_fd);
                return (1);
            }
            close(input_fd);
            if (execve(argv[0], argv, env) < 0)
            {
                    // std::cerr << RED << "Error executing CGI script."<< END << std::endl;
                    ft_send_error(500, response);
                    close(output_fd);
                    return (1);
            }
        }
        else
        {
            int status;

            sleep(1);
            int result = waitpid(response.pid, &status, WNOHANG);
            if(result < 0)
            {
                // std::cerr << SKY << "waitfeailed" << END << std::endl;
                ft_send_error(500, response);
                close(output_fd);
                return (1);
            }
            else  if (result && response.pid != -1)
            {

                cgi_response_content(response, name_output);

            }
            else
            {
                close(output_fd);
                return (0);
            }
            close(output_fd);
        }
    }
    return (1);
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