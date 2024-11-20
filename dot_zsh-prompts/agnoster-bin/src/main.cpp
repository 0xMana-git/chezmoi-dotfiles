#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <format>

using string = std::string;



#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}



string get_env(const string v) {

    const char* envstr = std::getenv(v.c_str());
    if(envstr == nullptr)
        return "";
    return string(envstr);
}

string CURRENT_BG = "NONE";


string CURRENT_FG = "";
constexpr string SEGMENT_SEPARATOR = "\ue0b0";

//TODO: properly implement this
/*
case ${SOLARIZED_THEME:-dark} in
    light) CURRENT_FG='white';;
    *)     CURRENT_FG='black';;
esac
*/

int exit_status = 0;
void init() {
    CURRENT_FG = "black";
    exit_status = atoi(exec_command("echo $?").c_str());
}

constexpr string context_bg = "24";





string prompt_segment(const string& bg_color = "", const string& fg_color = "", const string& arg2 = "") {
    string fg, bg, res;
    bg = bg_color.empty() ? "%k" : "%K{" + bg_color + "}";
    fg = fg_color.empty() ? "%f" : "%F{" + fg_color + "}";

    if(CURRENT_BG != "NONE" && bg_color != CURRENT_BG) {
        res = std::format(" %{{{}%F{{{}}}%}}{}%{{{}%}} ", bg, CURRENT_BG, SEGMENT_SEPARATOR, fg);
        //" %{$bg%F{$CURRENT_BG}%}$SEGMENT_SEPARATOR%{$fg%} "
    } else {
        res = std::format("%{{{}%}}%{{{}%}} ", bg, fg);
    }
    CURRENT_BG = bg_color;
    if(!arg2.empty()) 
        res += arg2;
    return res;
}

string prompt_segment_nospace(const string& bg_color = "", const string& fg_color = "", const string& arg2 = "") {
    string fg, bg, res;
    bg = bg_color.empty() ? "%k" : "%K{" + bg_color + "}";
    fg = fg_color.empty() ? "%f" : "%F{" + fg_color + "}";

    if(CURRENT_BG != "NONE" && bg_color != CURRENT_BG) {
        res = std::format("%{{{}%F{{{}}}%}}{}%{{{}%}}", bg, CURRENT_BG, SEGMENT_SEPARATOR, fg);
        //" %{$bg%F{$CURRENT_BG}%}$SEGMENT_SEPARATOR%{$fg%} "
    } else {
        res = std::format("%{{{}%}}%{{{}%}}", bg, fg);
    }
    CURRENT_BG = bg_color;
    if(!arg2.empty()) 
        res += arg2;
    return res;
}



string prompt_end() {
    string res;
    if(!CURRENT_BG.empty()) {
        res = " %{%k%F{" + CURRENT_BG + "}%}" + SEGMENT_SEPARATOR;
    }
    else {
        res = "%{%k%}";
    }
    res += "%{%f%}";
    CURRENT_BG = "";
    return res;
}

string prompt_context() {
    if(get_env("_USERNAME") != get_env("_DEFAULT_USER") || !get_env("SSH_CLIENT").empty()) {
        return prompt_segment(context_bg, "105", "%(!.%{%F{yellow}%}.)%n") +
        prompt_segment_nospace(context_bg, "39", "@") +
        prompt_segment_nospace(context_bg, "51", "%m");
    }
    return "";

}


string prompt_dir() {
    return prompt_segment("black", "81", "%~");
}


string prompt_status() {
    string symbols = "";
    if(get_env("_RETVAL") != "0") {
        symbols += "%{%F{red}%}✘";
    }
    if(get_env("_UID") == "0") {
        symbols += "%{%F{yellow}%}⚡";
    }
    if(atoi(exec_command("jobs -l | wc -l").c_str()) > 0) {
        symbols += "%{%F{cyan}%}⚙";
    }
    if(!symbols.empty())
        return prompt_segment(context_bg, "default", symbols);

    return "";
}


using prompt_callback_t = decltype(prompt_status);

prompt_callback_t* callbacks[] = {
    prompt_status,
    prompt_context,
    prompt_dir,
    prompt_end,
};

string build_prompt() {
    string res;
    for(prompt_callback_t* cb : callbacks) {
        res += cb();
    }
    return res;
}







int main(int argc, char** argv, char** envp) {
    init();
    std::cout << build_prompt();
}