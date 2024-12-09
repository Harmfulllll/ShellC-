#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<unistd.h>
#include<signal.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/wait.h>
#endif

#include<cerrno>
#include<stdlib.h>
#include<cstdlib>
#include<setjmp.h>
using namespace std;

/* 

 */
 volatile static sig_atomic_t sigint_flag = 0;
 static sigjmp_buf jmpbuf;

 vector<string>history;


// create builtin commands
vector<string> builtin_commands = {"cd", "clear", "history", "pwd", "exit", "help"};

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
    if(tokens[1].empty() || tokens.size()<2){
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
        perror("fork");
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
void exit_shell(){
     std::cout << "Exiting shell..." << std::endl;
    exit(0);
}
void add_history(string command){
   int curLength= history.size();
   if(curLength>=20){
    history.erase(history.begin());
   }
   history.push_back(command);
}

bool isNum(string str){
    for(int i=0;i<str.length();i++){
        if(!isdigit(str[i])){
            return false;
        }
    }
    return true;
}

void handle_history(vector<string>tokens){
    if(history.size()==0){
        cerr<<"history: no commands in history"<<endl;
        return;
    }
    if(tokens.size()==1){
        for(int i=0;i<history.size();i++){
           cout<<i+1<<" "<<history[i]<<endl;
       }  return;
    }
   if(tokens[1]=="-c"){
       // clear history
         history.clear();

   }else if(isNum(tokens[1])){
       int num= stoi(tokens[1]);
              
       if(num<0 || num>history.size()){
           cerr<<"history: invalid argument"<<endl;
       }else{
          for(int i=history.size()-num;i<history.size();i++){
              cout<<i+1<<" "<<history[i]<<endl;
          }
       }
   }

}
void handle_pwd(){
    char store[1024];
    if(getcwd(store, sizeof(store))!=NULL){
        cout<<store<<endl;
    }else{
        perror("getcwd() error");
    }
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

        if(command.length()>0 && ! isBlank(command))
        {  
          vector<string>tokens =  parse_input(command);

          add_history(command);
           
           for(int i=0;i<builtin_commands.size();i++ ){
                if(tokens[0] == builtin_commands[i]){
                     if(tokens[0]=="cd"){
                        handle_cd(tokens);
                     }
                     else if(tokens[0]=="clear"){
                        system("clear");
                        
                     }else if(tokens[0]=="history"){
                        handle_history(tokens);
                     }
                     else if(tokens[0]=="exit"){
                        exit_shell();
                     }else if(tokens[0]=="pwd"){
                        handle_pwd();
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