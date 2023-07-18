#include <sys/wait.h>
#include "../webserv.hpp"
#include "../config.hpp"
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <sys/time.h>
#include <signal.h>

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
            if (!out_file)
            {
                fill_response(500, response);
                ft_send_error(500, response);
            }
            std::string out = "out";
            out = generate_filename(out, &i);
            response.file_name_genarated.push_back(out);
            std::string line;  
            while(std::getline(out_file, line) && line != "\r")
            {
                int length = line.find(";") - (line.find(" ") + 1);
                std::string key = line.substr(0,line.find(" ")-1);
                std::string value = line.substr(line.find(" ")+ 1, length);
                if (key == "Content-type")
                {
                    if (*value.rbegin() == '\r')
                        value = value.substr(0, value.length() -1); 
                    add_extention_2(out ,value);
                }
                response.headers[key] = value;
            }
            std::ofstream file(out);
            if (!file)
            {
                fill_response(500, response);
                ft_send_error(500, response);
            }
            while (std::getline(out_file, line))
            {
                line += "\n";
                file << line;
            }
            response.path_file = out;
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

void delete_env(char **env)
{
    int i(0);
    while (env[i])
    {
        delete[] env[i++];
    }
    delete[] env[i];
    delete[] env;
}


int    execute_cgi(HttpResponse &response)
{
     int output_fd = 0;
    std::string name_output = "output";
    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    if (endTime.tv_sec - response.start == 35)
    {
        cgi_response_content(response, response.name_out);
        response.is_loop = 0;
        *response.close_connexion = true;
        kill(response.pid, SIGKILL);
        return (0);
    }
    if (response.pid == -1)
    {
        struct timeval startTime, endTime;
        gettimeofday(&startTime, NULL);
        response.start = startTime.tv_sec; 
        response.pid = fork();
        static int i;
        name_output = generate_filename(name_output, &i);
        response.name_out = name_output;
        output_fd = open(name_output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        response.file_name_genarated.push_back(name_output);
        char **env;
        env = get_env(response);
        if (response.pid == 0)
        {
            std::string path = response.path_file;
            std::string path_executable = response.cgi_it->cgi_pass;
            if ((response.cgi_it->file_extension == "php" && response.cgi_it->cgi_pass != "response/cgi-bin/php-cgi")
                || (response.cgi_it->file_extension == "py" && response.cgi_it->cgi_pass != "/usr/bin/python"))
            {
                ft_send_error(500, response);
                close(output_fd);
                return(1);
            }
            int input_fd  = 0;
            if (response.request.method == "POST")
                input_fd = open("cgi.txt", O_RDONLY , 0666);
            char* const argv[] = {(char*)path_executable.c_str(), (char*)path.c_str(), NULL};
            if (dup2(output_fd, 1) == -1 || dup2(output_fd, 2) == -1) 
            {
                std::cerr << "Error duplicating file descriptor." << std::endl;
                *response.close_connexion = true;
                ft_send_error(500, response);
                close(output_fd);
                return(1);
            }
            close(output_fd);
            if (input_fd > 0 && dup2(input_fd, STDIN_FILENO) < 0) 
            {
                *response.close_connexion = true;
                ft_send_error(500, response);
                close(input_fd);
                return (1);
            }
            close(input_fd);
            if (execve(argv[0], argv, env) < 0)
            {
                *response.close_connexion = true;
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
            gettimeofday(&endTime, NULL);
            
            if(result < 0)
            {
                *response.close_connexion = true;
                ft_send_error(500, response);
                delete_env(env);
                close(output_fd);
                return (1);
            }
            else  if (result && response.pid != -1)
                cgi_response_content(response, name_output);
            else
            {
                response.is_loop = 1;
                close(output_fd);
                delete_env(env);
                return (0);
            }
        }
        delete_env(env);
        close(output_fd);
    }
    return (1);
}
