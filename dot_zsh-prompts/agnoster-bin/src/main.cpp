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
#include <filesystem>
#include <git2.h>


constexpr bool USE_LIBGIT = false;

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

void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
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
//constexpr string PL_BRANCH_CHAR = "\ue0a0";

//TODO: properly implement this
/*
case ${SOLARIZED_THEME:-dark} in
    light) CURRENT_FG='white';;
    *)     CURRENT_FG='black';;
esac
*/
constexpr string context_bg = "24";

void init() {
    CURRENT_FG = "black";
    if(USE_LIBGIT)
        git_libgit2_init();
}







inline string prompt_segment(const string& bg_color = "", const string& fg_color = "", const string& arg2 = "") {
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

inline string prompt_segment_nospace(const string& bg_color = "", const string& fg_color = "", const string& arg2 = "") {
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


inline string prompt_git() {
    
    // if(!std::filesystem::exists(".git"))
    //     return "";
    git_repository *repo = nullptr;
    string mode, ref, repo_path, ahead = "", behind = "";;

    if(USE_LIBGIT) {
        
        int error = git_repository_open(&repo, ".");
        if(error < 0)
            return "";
    } else {
        repo_path = exec_command("git rev-parse --git-dir 2>/dev/null");
        if(repo_path.size() == 0) {
            return "";
    }
    }
    

    string res = prompt_segment("green", CURRENT_FG);
    string PL_BRANCH_CHAR = "\ue0a0";
    
    
    //repo_path = git_repository_path(repo);
    
    //

    
    
    
    //I actually never free resources but its because it
    //doesnt matter
    
    
    
    //string ahead = exec_command("git log --oneline @{upstream}.. 2>/dev/null");
    //string behind = exec_command("git log --oneline ..@{upstream} 2>/dev/null");
    if(USE_LIBGIT) {
        git_reference* git_ref_outer, *git_ref;
        int error = git_reference_lookup(&git_ref_outer, repo, "HEAD");

        error = git_reference_resolve(&git_ref, git_ref_outer);
        ref = git_reference_name(git_ref);
    } else {
        ref = exec_command("git symbolic-ref HEAD 2> /dev/null");
    }
   
    ref = ref.size() != 0 ? ref : "◈ " + exec_command("git descriexport_initbe --exact-match --tags HEAD 2> /dev/null");
    ref = ref.size() != 0 ? ref : "➦ " + exec_command("git rev-parse --short HEAD 2> /dev/null");
    if(ahead.size() != 0 && behind.size() != 0)
        PL_BRANCH_CHAR = "\u21c5";
    else if(ahead.size() != 0) 
        PL_BRANCH_CHAR = "\u21b1";
    else if(behind.size() != 0) 
        PL_BRANCH_CHAR = "\u21b0";
    if(std::filesystem::exists(repo_path + "/BISECT_LOG"))
        mode = " <B>";
    else if(std::filesystem::exists(repo_path + "/MERGE_HEAD")) 
        mode = " >M<";
    else if(std::filesystem::exists(repo_path + "/rebase") ||
    std::filesystem::exists(repo_path + "/rebase-apply") ||
    std::filesystem::exists(repo_path + "/rebase-merge") ||
    std::filesystem::exists(repo_path + "/../.dotest"))
        mode = " >R>";
    

    replace_all(ref, "refs/","");
    replace_all(ref, "heads/","");
    res += PL_BRANCH_CHAR + " ";
    res += ref;
    res += get_env("_VCS_INFO_MSG_0");
    res += mode;
    
    return res;
}


inline string prompt_end() {
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

inline string prompt_context() {
    if(get_env("_USERNAME") != get_env("_DEFAULT_USER") || !get_env("SSH_CLIENT").empty()) {
        return prompt_segment(context_bg, "105", "%(!.%{%F{yellow}%}.)%n") +
        prompt_segment_nospace(context_bg, "39", "@") +
        prompt_segment_nospace(context_bg, "51", "%m");
    }
    return "";

}


inline string prompt_dir() {
    return prompt_segment("black", "81", "%~");
}

inline string prompt_status() {
    string symbols = "";
    if(get_env("_RETVAL") != "0") {
        symbols += "%{%F{red}%}✘";
    }
    if(get_env("_UID") == "0") {
        symbols += "%{%F{yellow}%}⚡";
    }
    //if(atoi(exec_command("jobs -l | wc -l").c_str()) > 0) {symbols += "%{%F{cyan}%}⚙";}
    if(!symbols.empty())
        return prompt_segment(context_bg, "default", symbols);

    return "";
}


using prompt_callback_t = decltype(prompt_status);

prompt_callback_t* callbacks[] = {
    prompt_status,
    prompt_context,
    prompt_dir,
    prompt_git,
    prompt_end,
};

string build_prompt() {
    string res = prompt_status();
    res += prompt_context();
    res += prompt_dir();
    res += prompt_git();
    res += prompt_end();
    
    return res;
}







int main(int argc, char** argv, char** envp) {
    init();

    //git_libgit2_shutdown();
    std::cout << build_prompt();
}