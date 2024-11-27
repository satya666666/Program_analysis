#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<signal.h>
#include<syscall.h>
#include<errno.h>
#include<vector>
#include<map>
#include<iostream>
#include<string>
#include<sstream>
#include<unistd.h>

#include<sys/ptrace.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/user.h>
#include<sys/reg.h>
#include<sys/personality.h>
#include<sys/stat.h>

using namespace std;

/*

 `./debugger path/to/executable`

######### Commads #########

next/ nexti         : To step to next instruction
break 0x123456      : To set a break point at 0x123456
continue / c        : To continue the execution, even c can be used
exit                : To exit the program
infobreak/ i b      : List of break points
info registers/ i r : List of registers with their values


############################ 
*/

map<unsigned int, unsigned int> Breakpoints;

void execute_program(pid_t child, const char* prog_name){
    
    // Disable ASLR on the executable
    personality(ADDR_NO_RANDOMIZE);

    // Trace the execution
    if(ptrace(PTRACE_TRACEME, child, 0, 0)<0)
    {
        cerr<<"[!] error in PTRACE_TRACEME";
        return;
    }

    // pid_t curr_pid = getpid();
    // cout<<"pid in execute_program fn is "<<curr_pid;

    execl(prog_name, prog_name, nullptr);

}



void disassemble( pid_t child) {
    int wait_status;
    pid_t new_child = fork();

    if (new_child == -1) {
        perror("fork failed");
        return;
    }

    if (new_child == 0) {
        // In child process
        char mypid[16];
        sprintf(mypid, "%d", child);
        printf("\n\n##########   Disassemble Menu #########\n\n1. Get info about targetted function\n2. Full Program\n\n");
        // scanf("Enter your Choice %d",&choice);
       string input;
    int choice;

    cout << "Enter your choice: ";
    getline(cin, input);  // Get the entire input as a string

    // Convert the string to an integer
    stringstream(input) >> choice;
        if(choice==1){
            string getFunctionName;
            printf("Enter target function name ");
            getline(cin,getFunctionName);
            string comm ="objdump --disassemble="+getFunctionName+" /proc/"+string(mypid)+"/exe";
            printf("Command: %s\n", comm.c_str());
            system(comm.c_str());
        }else{
            string comm = "objdump -d /proc/" + string(mypid) + "/exe";
            printf("Command: %s\n", comm.c_str());
            system(comm.c_str());
        }
      
       
        exit(0);  // Ensure child exits
    } else {
        // In parent process
        waitpid(new_child, &wait_status, 0);  // Wait for child to finish
    }
}

void Continue(pid_t child) {
    int wait_status;
    struct user_regs_struct regs;

    // Continue execution (this will trigger breakpoint if any)
    ptrace(PTRACE_CONT, child, 0, 0);
    wait(&wait_status);

    if (WIFSTOPPED(wait_status)) {
        // Retrieve current register state
        ptrace(PTRACE_GETREGS, child, 0, &regs);

        // Check if breakpoint is hit by examining the signal number
       if (WSTOPSIG(wait_status) == SIGTRAP) {
    printf("[+] Breakpoint hit at 0x%08llx\n", regs.rip);

    // Calculate address
    unsigned int addr = regs.rip - 1;
    printf("[DEBUG] Current RIP: 0x%08llx, Breakpoint Address: 0x%08x\n", regs.rip, addr);

    auto iter = Breakpoints.find(addr);
    if (iter != Breakpoints.end()) {
        unsigned int original_data = iter->second;
        printf("[++] Restoring data at 0x%08x: 0x%08x\n", iter->first, iter->second);

        // Restore instruction
        if (ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)original_data) == -1) {
            perror("[!] Error restoring original instruction");
            return;
        }

        // Adjust RIP
        regs.rip -= 1;
        printf("[DEBUG] Adjusted RIP: 0x%08llx\n", regs.rip);

        if (ptrace(PTRACE_SETREGS, child, 0, &regs) == -1) {
            perror("[!] Error setting registers");
            return;
        }

        // Remove breakpoint
        Breakpoints.erase(addr);
        printf("[+] Breakpoint restored successfully.\n");
    } else {
        printf("[!] No matching breakpoint found.\n");
    }
    } else if (WSTOPSIG(wait_status) == SIGSEGV) {
        printf("[!] SIGSEGV occurred at RIP: 0x%08llx\n", regs.rip);
        // Optionally inspect the memory state or terminate
    } else {
        printf("[!] Unexpected signal received: %d\n", WSTOPSIG(wait_status));
    }


        // Now continue execution
        // ptrace(PTRACE_CONT, child, 0, 0);
    } else {
        printf("[!] Unexpected wait status: %d\n", wait_status);
    }

   
}


// Handle next command
#define MAX_EXECUTED_INSTRUCTIONS 100000 
unsigned long executed_instructions[MAX_EXECUTED_INSTRUCTIONS];
int instruction_count = 0;

void nexti(pid_t child) {
    int wait_status;
    struct user_regs_struct regs;

    if (ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0) {
        perror("[!] Error in PTRACE_SINGLESTEP");
        return;
    }

    wait(&wait_status);

    if (WIFSTOPPED(wait_status)) {
        ptrace(PTRACE_GETREGS, child, 0, &regs);

        // Check if the current RIP is already in the executed list
        for (int i = 0; i < instruction_count; i++) {
            if (executed_instructions[i] == regs.rip) {
                printf("[!] Detected loop at 0x%08llx\n", regs.rip);
                // Optionally skip the loop or take other actions
                return;
            }
        }
        // Add the current RIP to the executed instructions list
        if (instruction_count < MAX_EXECUTED_INSTRUCTIONS) {
            executed_instructions[instruction_count++] = regs.rip;
        } else {
            printf("[!] Executed instructions limit reached. Unable to track further.\n");
        }

        printf("[+] Single step completed at 0x%08llx\n", regs.rip);
    } else {
        printf("[!] Unexpected wait status: %d\n", wait_status);
    }
}



void Set_breakpoint(pid_t child, unsigned int addr){

    // To set a breakpoint to the given address write 'INT 3' trap instruction to the address
    // 0xcc is the opcode for INT 3 
    // peek the data in the addr and then poke it with the 'INT 3/ 0xCC' modified data

    unsigned int data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    printf("-----------------------------------\n");
    printf("[++] Data at 0x%08x: 0x%08x\n",addr,data);
    unsigned int data_with_trap = (data & 0xffffff00 ) | 0xcc;
    ptrace(PTRACE_POKETEXT, child, (void *)addr, (void *)data_with_trap);
    printf("[+] Breakpoint set at 0x%08x\n", addr);

    unsigned afterEdit=ptrace(PTRACE_PEEKTEXT,child,(void *)addr,0);
    printf("[++] After Trap, Data at 0x%08x: 0x%08x\n",addr,afterEdit);
    printf("-----------------------------------\n");

    // Insert this to the map

    Breakpoints.insert(std::pair<unsigned int, unsigned int> (addr, data));

}


void Info_registers(pid_t child){

    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, child, 0, 0);



    printf("######## Regsiters Info ########\n");
    printf(" RAX:  0x%08llx\n RBX:  0x%08llx\n RCX:  0x%08llx\n RDX:  0x%08llx\n RSP:  0x%08llx\n RBP:  0x%08llx\n RSI:  0x%08llx\n RDI:  0x%08llx\n RIP:  0x%08llx\n"
    , regs.rax,regs.rbx,regs.rcx,regs.rdx,regs.rsp,regs.rbp,regs.rsi,regs.rdi,regs.rip );

}


void List_breakpoints(){
    
    printf("\n   ######## Breakpoints ######## \n\n");
    int i =0;
    for(auto bp : Breakpoints){

        printf("-> Breakpoint %d : 0x%08x\n", ++i, bp.first);

    }

    printf("\n");
}


string last_command;
void handle_command(pid_t child, string command){
    if(command == "exit"){       
        kill(child, SIGKILL);
        printf("\n ######## Debugger Stopped ######## \n");
        exit(0);
    }

    else if(command.find("disass")==0 || command.find("dis")==0){
      
        disassemble( child);
    }
        
    else if(command == "continue" || command == "c") 
        Continue(child); 
    
    else if(command == "next" || command == "nexti")
        nexti(child);

    else if(command.find("break") == 0){

        // Get the last string (hopefully the address- 0x#####) // break 0x#####
        istringstream iss(command);

        std::size_t Lastword = command.find_last_of(' ');
        string addr = command.substr(Lastword+1);


        // convert the addr from string to unsigned int
        unsigned int uiaddr;
        std::stringstream hexstring;
        hexstring << std::hex << addr;
        hexstring >> uiaddr;

        //printf("----Breakpoint set at 0x%08x------\n", uiaddr);

        Set_breakpoint(child, uiaddr);

    }

    else if(command == "info registers" || command == "i r"){
        //cout<<command;
        Info_registers(child);
    }

    else if(command == "info breakpoints" || command == "i b"){
        List_breakpoints();
    }

    else if(command.empty()){
        last_command = command;
        nexti(child);
    }

    else{
        printf("[!] Unkonown command entered\n");
        command = "";
    }

    last_command = command;
    return;
    
}
int main(int argc, char **argv){
    
    int wait_status;
    
    char * executable;
    string command;

    if(argc < 2){
        cout << "\nPath to a executable file was not provided \n"<<"\nUsage: ./debugger path/to/executable/file \n";
        return -1;
    }
    executable = argv[1];

    // check if the provided file exists in the given path
    struct stat info;
    if(stat(executable, &info)!=0)
    {
        printf("\n '%s' file doesn't exist, please provide a proper file \n", executable);
        return -1;
    }


    pid_t child = fork();

    // pid_t cur_pid = getpid();
    // cout<<"Pid in main fn is"<<cur_pid;

    // execute the binary in child process
    printf("----------%d",child);
    if(child == 0){
        execute_program(child, executable);
    }

    // start debugging session
    printf("\nChild process running : %d\n\n",child);
    printf("\n ######## Debugger sdistarted ######## \n\n");
    wait(&wait_status);


    while (true) {
        // Handle cases when the program exits or stops
        if (WIFEXITED(wait_status)) {
            printf("\n ######## Target Program Finished Execution ######## \n");
            break;
        }

        if (WIFSTOPPED(wait_status)) {
            // Get all register values
            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, child, 0, &regs);

            // Extract value of RIP register, which contains the address of the current instruction
            unsigned instr = ptrace(PTRACE_PEEKTEXT, child, regs.rip, 0);
            printf("0x%08llx >> ", regs.rip);

            // Automatically handle "next" command to step through instructions
            // prompt for a new command
            getline(cin, command);

       
            // Handle the new command
            handle_command(child, command);

            // Continue execution and wait for the next event
            // ptrace(PTRACE_CONT, child, 0, 0);
            // wait(&wait_status);
        } else {
            printf("[!] Unexpected wait status: %d\n", wait_status);
            break;
        }
    }

}