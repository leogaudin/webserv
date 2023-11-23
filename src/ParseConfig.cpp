//
// Created by Jessica Chamak on 11/23/23.
//

#include "../include/Webserv.hpp"

Config *conf = new Config[MAX_SERVERS];
int w = 0;

std::string *fill_arg(std::string string)
{
    std::string words[100];
    std::string* sentence = new std::string[1000];
    std::string tmp;
    int i = 0;
    int j = 0;
    int k = 0;
    while (string[i])
    {
        while (string[i] && (string[i] == ' ' || string[i] == '\t' || string[i] == '\n'))
            i ++;
        while (string[i] && string[i] != ' ' && string[i] != '\t' && string[i] != '\n')
        {
            tmp += string[i];
            i++;
        }
        words[j] = tmp;
        sentence[k] = words[j];
        tmp.clear();
        j ++;
        k ++;
    }
    return (sentence);
}

void whatType2(int type, std::string **done, int i, int j, std::string name)
{
    if (type == 0)
        conf[w]._locations[name]._listen = atoi(done[i][j].c_str());
    else if (type == 1)
        conf[w]._locations[name]._host = done[i][j];
    else if (type == 2)
        conf[w]._locations[name]._serverName = done[i][j];
    else if (type == 3)
        conf[w]._locations[name]._root = done[i][j];
    else if (type == 4)
        conf[w]._locations[name]._index = done[i][j];
    else if (type == 5)
        conf[w]._locations[name]._client_max_body_size = atoi(done[i][j].c_str());
    else if (type == 6)
    {
        int k = 0;
        while (!done[i][k].empty())
            k ++;
        while (!done[i][j].empty())
        {
            conf[w]._locations[name]._errorPages[atoi(done[i][j].c_str())] = done[i][k - 1];
            j ++;
        }
    }
    else if (type == 7)
    {
        while (!done[i][j].empty())
        {
            conf[w]._locations[name]._cgiExtensions.push_back(done[i][j]);
            j++;
        }
    }
    else if (type == 8)
    {
        while (!done[i][j].empty())
        {
            conf[w]._locations[name]._cgiPaths.push_back(done[i][j]);
            j++;
        }
    }
    else if (type == 9)
    {
        if (done[i][j] == "off")
            conf[w]._locations[name]._autoindex = 0;
        else if (done[i][j] == "on")
            conf[w]._locations[name]._autoindex = 1;
    }
    else if (type == 10)
    {
         while (!done[i][j].empty()) {
             Method current;
             if (!done[i][j].compare("GET"))
                 current = GET;
             else if (!done[i][j].compare("POST"))
                 current = POST;
             else if (!done[i][j].compare("DELETE"))
                 current = DELETE;
             else
                 current = UNKNOWN;
            conf[w]._locations[name]._allowedMethods.push_back(current);
            j++;
         }
    }
    else if (type == 11)
        conf[w]._locations[name]._redirect = done[i][j];
    else if (type == 12)
        conf[w]._locations[name]._alias = done[i][j];
}

void fill_conf2(std::string **done, int i, int j, std::string name)
{
    int k = 0;
    std::string type[] = {"listen", "host", "server_name", "root", "index",
                          "client_max_body_size", "error_page", "cgi_ext",
                          "cgi_path", "autoindex", "allow_methods", "redirect", "alias"};
    while (!done[i][j].empty() && done[i][j]!= "}")
    {
        while (k < 13)
        {
            if (done[i][j] == type[k])
                whatType2(k, done, i, j + 1, name);
            k ++;
        }
        i ++;
        j = 0;
        k = 0;
    }
}

void whatType(int type, std::string **done, int i, int j)
{
    if (type == 0)
        conf[w]._listen = atoi(done[i][j].c_str());
    else if (type == 1)
        conf[w]._host = done[i][j];
    else if (type == 2)
        conf[w]._serverName = done[i][j];
    else if (type == 3)
        conf[w]._root = done[i][j];
    else if (type == 4)
        conf[w]._index = done[i][j];
    else if (type == 5)
        conf[w]._client_max_body_size = atoi(done[i][j].c_str());
    else if (type == 6)
    {
        int k = 0;
        while (!done[i][k].empty())
            k ++;
        while (!done[i][j].empty())
        {
            conf[w]._errorPages[atoi(done[i][j].c_str())] = done[i][k - 1];
            j ++;
        }
    }
    else if (type == 7)
    {
        conf[w]._locations[done[i][j]] =  conf[w].toLocationConfig();
        fill_conf2(done, i, j + 2, done[i][j]);
    }
    else if (type == 8)
    {
        while (!done[i][j].empty())
        {
            conf[w]._cgiExtensions.push_back(done[i][j]);
            j++;
        }
    }
    else if (type == 9)
    {
        while (!done[i][j].empty())
        {
            conf[w]._cgiPaths.push_back(done[i][j]);
            j++;
        }
    }
}

void fill_conf(std::string **done)
{
    int i = 0;
    int j = 0;
    int k = 0;
    std::string type[] = {"listen", "host", "server_name", "root", "index",
                          "client_max_body_size", "error_page", "location", "cgi_ext",
                          "cgi_path", "autoindex", "allow_methods", "redirect", "alias"};
    if (done[i][1] == type[7])
        j = 3;
    while (!done[i][j].empty())
    {
        while (k < 14)
        {
            if (done[i][j] == type[k])
            {
                whatType(k, done, i, j + 1);
                break;
            }
            k ++;
        }
        i ++;
        j = 0;
        if (k == 7)
        {
            while (!done[i][j].empty() && done[i][j]!= "}")
                i ++;
            j ++;
        }
        k = 0;
    }
}

void write_conf(std::string **done)
{
    int i = 0;
    int j = 0;
    while (!done[i][j].empty())
    {
        while (!done[i][j].empty())
        {
            std::cout << done[i][j] << " ";
            j ++;
        }
        std::cout << std::endl;
        i ++;
        j = 0;
    }
    fill_conf(done);
}

int isolation(std::string **paragraph, int i)
{
    int closing = 0;
    int opening = 0;
    int j;
    int m = 0;
    int n = 0;
    std::string **done = new std::string*[300];
    for(j = 0; j < 300; ++j)
        done[j] = new std::string[1000];
    j = 0;
    while (paragraph[i][j] == "server" || paragraph[i][j] == "{" || paragraph[i][j] == "}")
        j ++;
    while (!paragraph[i][j].empty())
    {
        done[m][n] = paragraph[i][j];
        n ++;
        j++;
    }
    opening += 1;
    j = n = 0;
    i++;
    m ++;
    while (!paragraph[i][j].empty())
    {
        while (!paragraph[i][j].empty())
        {
            if (paragraph[i][j] == "{")
                opening += 1;
            else if (paragraph[i][j] == "}")
                closing += 1;
            if (opening == closing)
                break;
            done[m][n] = paragraph[i][j];
            n ++;
            j++;
        }
        if (opening == closing)
            break;
        i++;
        m ++;
        j = n = 0;
    }
    // write_conf(done);
    fill_conf(done);
    return (i);
}

Config * parseConfig(std::string filename)
{
    std::ifstream file;
    std::string string;
    std::string **paragraph = new std::string*[300];
    for(int i = 0; i < 300; ++i)
        paragraph[i] = new std::string[1000];
    file.open(filename);
    int i = 0;
    int j = 0;
    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, string, ';');
            while (!(*(fill_arg(string) + i)).empty())
            {
                paragraph[j][i] = *(fill_arg(string) + i);
                i++;
            }
            i = 0;
            j ++;
        }
    }
    i = 0;
    j = 0;
    while (!paragraph[i][0].empty())
    {
        while (!paragraph[i][j].empty())
            j ++;
        i ++;
        j = 0;
    }
    i = 0;
    while (!paragraph[i + 1][j].empty())
    {
        i = isolation(paragraph, i);
        conf[w]._parsed = true;
//        std::cout << "w est " << w << std::endl;
        // conf[w].printConfig(0);
        w ++;
    }
    return (conf);
}