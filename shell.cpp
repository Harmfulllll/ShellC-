#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<cerrno>
#include<stdlib.h>
#include<cstdlib>
#include<setjmp.h>
using namespace std;

/* 

 */
 volatile static sig_atomic_t sigint_flag = 0;
 static sigjmp_buf jmpbuf;


// create builtin commands
vector<string> builtin_commands = {"cd"};

// Function to read input from the user
string read_input(){
    string input;
    getline(cin, input);
    if(cin.eof()){
        exit(EXIT_SUCCESS);
    }
    return input;
}

void cleanup(){}

bool isBlank(string str){
    for(int i=0;i<str.length();i++){
         if(str[i]!=' '){
            return false;
         }
    } return true;
}

vector<string>parse_input( string command){
     vector<string>tokens;
     string token;

     istringstream ss(command);
        while(ss>>token){
            tokens.push_back(token);
        }
     token.clear();
     return tokens;
}

void handle_cd(vector<string>tokens){
    if(tokens[1].empty()){
        cerr<<"cd: missing argument"<<endl;
        exit(EXIT_FAILURE);
    }else if(chdir(tokens[1].c_str())!=0){
        perror("cd");
    }
    else {
        cout<<"Directory changed to "<<tokens[1]<<endl;
    }
}
void execute(vector<string>tokens){
    pid_t pid= fork();
    int start_loc;
    if(pid==0){
           // child process         
         

         vector<char*>args;
         for(int i=0;i<tokens.size();i++){
            args.push_back(const_cast<char*>(tokens[i].c_str()));
         }
         args.push_back(nullptr);
         execvp(args[0],args.data());
         /* following won't run if execution successfull */
         perror("execvp");
         exit(EXIT_FAILURE);
        
    }else if(pid<0){
        // fork failed
        perror('fork');
        exit(1);
     }else{
        do{
            waitpid(pid, &start_loc, WUNTRACED) ;
        }while(!WIFEXITED(start_loc) && !WIFSIGNALED(start_loc));
     }
    }
void handle_signal(int sigNUM){
    if(!sigint_flag){
       return;
    }
     
     siglongjmp(jmpbuf, 10);
}

int main(){
    // initialize
        
        signal(SIGINT, handle_signal);

    while(true){

        if(sigsetjmp(jmpbuf,1)==10){
            cout<<"Restarting shell"<<endl;
        }
        sigint_flag = 1;
        cout<< ">>";
        string command;
        command = read_input();

        if(command.empty()){
           cleanup();
           continue;
        }
        if(command == "exit"){
            break;
        }
        if(command.length()>0 && ! isBlank(command))
        {  
          vector<string>tokens =  parse_input(command);
           
           for(int i=0;i<builtin_commands.size();i++ ){
                if(tokens[0] == builtin_commands[i]){
                     if(tokens[0]=="cd"){
                        handle_cd(tokens);
                        continue;
                     }
                }
           }
             
             execute(tokens);

           

        } 
     }
     
    
    // Cleanup
    cleanup();
    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;

} 