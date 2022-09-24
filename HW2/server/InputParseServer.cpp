//#include "InputParseServer.h"
/*
    C Language Header for Socket Programming
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
/*
    C++ Language Header for Input Parse
*/
#include <iostream>
#include <map>
#include <memory>
#include <stack>

using namespace std;

namespace Operator {
map<char, int> operatorMap = {
    {'+', 1},
    {'-', 1},
    {'*', 2},
    {'/', 2},
    {'(', 3},
    {')', 4}};
}
class InfixToPostfix {
   private:
    stack<char> mStack = stack<char>();
    string resultExp;
    void pushOperator(const char &op) {
        // op is Valid Key
        int top = Operator::operatorMap[mStack.top()];
        int cur = Operator::operatorMap[op];
        if (Operator::operatorMap[')'] == cur) {
            while (top != Operator::operatorMap['(']) {
                // cout << mStack.top();
                resultExp += mStack.top();
                mStack.pop();
                if (!mStack.empty()) {
                    top = Operator::operatorMap[mStack.top()];
                } else {
                    return;
                }
                continue;
            }
            mStack.pop();
            return;
        }
        while (top >= cur) {
            if (Operator::operatorMap['('] == top) {
                mStack.push(op);
                return;
            }
            // cout << mStack.top();
            resultExp += mStack.top();
            mStack.pop();
            if (!mStack.empty()) {
                top = Operator::operatorMap[mStack.top()];
                continue;
            } else {
                break;
            }
        }
        mStack.push(op);
        return;
    }

   public:
    string infixToPostfix(string &infix) {
        for (char &s : infix) {
            if (isOperator(static_cast<const char &>(s))) {  // is Operator!
                if (mStack.empty()) {                        // if Stack is Empty push
                    mStack.push(s);
                } else {  // if Stack is Not Empty
                    pushOperator(static_cast<const char &>(s));
                }
            } else {  // is Not Operator
                resultExp += s;
            }
        }
        while (!mStack.empty()) {
            // cout << mStack.top();
            resultExp += mStack.top();
            mStack.pop();
        }
        return resultExp;
    }
    static bool isOperator(const char &c) {
        if (Operator::operatorMap.count(c)) {  // if is Operator
            return true;
        } else {
            return false;  // if is Not Operator
        }
    }
};

class Node {
   private:
    int charToInt(const char &c) {
        return c - 48;
    }

   public:
    char mexp;
    Node *mleft;
    Node *mright;
    static char intToChar(const int &n) {
        return n + 48;
    }
    Node() = default;
    Node(const char &exp) {
        if (InfixToPostfix::isOperator(exp)) {
            mexp = exp;
        } else {
            mexp = charToInt(exp);
        }
    }
};



class ListenTCPServer {
   private:
    const int listenInputParsePort = 10003;
    const int sendAddSubPort = 10001;
    const int sendMulDivPort = 10002;
    const int backlog = 10;
    const int maxDataSize = 100;
    int socketFd, newFd;          /* listen on sockfd, new connection on newfd */
    struct sockaddr_in myAddr;    /* my address information */
    struct sockaddr_in theirAddr; /* connector's address information */
    socklen_t sin_size;
    int numbytes;
    const char *serverAddress = "127.0.0.1";
    string createPayload(const float &leftOperand, const float &rightOperand, const char &op) {
        return to_string(leftOperand) + " " + to_string(op) + " " + to_string(rightOperand);
    }

    string getData(const int &port, const char *payload, int size) {
        struct sockaddr_in their_addr; /* connector's address information */
        if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }
        their_addr.sin_family = AF_INET;   /* host byte order */
        their_addr.sin_port = htons(port); /* short, network byte order */
        their_addr.sin_addr.s_addr = inet_addr(serverAddress);
        bzero(&(their_addr.sin_zero), 8); /* zero the rest of the struct */

        if (connect(socketFd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }
        if (send(socketFd, payload, size, 0) == -1) {
            perror("send");
        }
        char buf[maxDataSize];
        if ((numbytes = recv(socketFd, buf, maxDataSize, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';
        close(socketFd);
        return string(buf);
    }

   protected:
    float getAddSubData(const float &leftOperand, const float &rightOperand, const char &op) {
        cout << "exp" <<leftOperand << op << rightOperand<<endl;
        string &&exp = createPayload(leftOperand, rightOperand, op);
        string &&res = getData(sendAddSubPort, exp.c_str(), exp.size());
        cout << res << endl;
        return stof(res);
    }
    float getMulDivData(const float &leftOperand, const float &rightOperand, const char &op) {
        string exp = createPayload(leftOperand,rightOperand,op);
        string res = getData(sendMulDivPort, exp.c_str(), exp.size());
        cout << res << endl;
        return stof(res);
    }

   public:
    void startListen(auto makeTreeModule) {
        if ((socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {  // 1. Create TCP Socket
            perror("socket");
            exit(1);
        }

        myAddr.sin_family = AF_INET;                                                    /* host byte order */
        myAddr.sin_port = htons(listenInputParsePort); /* short, network byte order */  // 2. Set Server Port 3490
        myAddr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */                 // 3. Set Server IP Address
        bzero(&(myAddr.sin_zero), 8);                                                   /* zero the rest of the struct */

        socklen_t reuseAddress = 1;
        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseAddress, sizeof(reuseAddress)) == -1) {
            perror("setsockopt");  // Port Reuse Option!
        }

        if (bind(socketFd, (struct sockaddr *)&myAddr, sizeof(struct sockaddr)) == -1) {  // 4. Bind IP, Port
            perror("bind");
            exit(1);
        }

        if (listen(socketFd, backlog) == -1) {  // 5. Listen! => TCP 3단계 Connection
            perror("listen");
            exit(1);
        }

        while (1) { /* main accept() loop */
            sin_size = sizeof(struct sockaddr_in);
            if ((newFd = accept(socketFd, (struct sockaddr *)&theirAddr,
                                &sin_size)) == -1) {  // 6. Accept!
                perror("accept");
                continue;
            }
            cout << "server: got connection from " << inet_ntoa(theirAddr.sin_addr) << endl;
            if (!fork()) { /* this is the child process */  // 7. then, Child Process do Any Jobs..
                cout << getpid() << endl;
                char buf[maxDataSize];
                if ((numbytes = recv(newFd, buf, maxDataSize, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }
                buf[numbytes] = '\0';
                cout << "Input: "<<buf <<endl;
                string userInput(buf);
                auto infixToPostModule = make_unique<InfixToPostfix>();
                string tmp = infixToPostModule->infixToPostfix(userInput);
                //cout << tmp << endl;
                const Node* postRoot = makeTreeModule->makeExpTree(tmp);
                makeTreeModule->postOrder(postRoot);
                string result = to_string(makeTreeModule->calc(postRoot));
                //cout << "result!!" <<<<endl;
                //string result = to_string(makeTreeModule->calc(postRoot));

                // TODO Jobs..
                
                if (send(newFd, result.c_str(),result.size(), 0) == -1)
                     perror("send");
                cout << "Conncetion Successful! Message! :" << buf << endl;
                close(newFd);
                exit(0);
            }
            close(newFd); /* parent doesn't need this */
            while (waitpid(-1, NULL, WNOHANG) > 0)
                ; /* clean up child processes */
        }
    }
};

class MakeExpTree : ListenTCPServer {
   private:
    stack<Node *> mStack = stack<Node *>();
    const Node *mRoot = nullptr;
    auto createNode(const char &exp) {
        auto node = new Node(exp);
        node->mleft = nullptr;
        node->mright = nullptr;
        return node;
    }

    auto makeTree(const char &op, Node *const leftNode, Node *const rightNode) {
        auto rootNode = createNode(op);
        rootNode->mleft = leftNode;
        rootNode->mright = rightNode;
        return rootNode;
    }

    void freeTree(const Node *root) {
        if (root) {
            freeTree(root->mleft);
            freeTree(root->mright);
            if (InfixToPostfix::isOperator(root->mexp)) {
                cout << root->mexp << " ";
                delete root;
            } else {
                cout << Node::intToChar(root->mexp) << " ";
                delete root;
            }
        }
    }

   public:
    ~MakeExpTree() {
        freeTree(mRoot);
    }
    const Node *makeExpTree(const string &postFix) {
        for (const char &s : postFix) {
            if (InfixToPostfix::isOperator(static_cast<const char &>(s))) {  // if Is Operator
                Node *rightNode = mStack.top();
                mStack.pop();
                Node *leftNode = mStack.top();
                mStack.pop();
                mStack.push(makeTree(s, leftNode, rightNode));
            } else {  // if is Not Operator
                Node *operandNode = createNode(s);
                mStack.push(operandNode);
            }
        }
        mRoot = mStack.top();
        mStack.pop();
        return mRoot;
    }
    void postOrder(const Node* root) {
        if(!root){
            postOrder(root->mleft);
            postOrder(root->mright);
            cout << root->mexp << " ";
        }
    }

    float calc(const Node *const root) {
        if (root->mleft == nullptr && root->mright == nullptr)
            return root->mexp;
        float leftOperand = calc(root->mleft);
        float rightOperand = calc(root->mright);
        switch (root->mexp) {
            case '+':
                //cout<<"test "<< leftOperand << root->mexp <<rightOperand<<endl;
                return ListenTCPServer::getAddSubData(leftOperand, rightOperand, '+');  // leftOperand + rightOperand;  // Call Server
            case '-':
                //cout<<"test "<< leftOperand << root->mexp <<rightOperand<<endl;
                return ListenTCPServer::getAddSubData(leftOperand, rightOperand, '-');  // Call Server
            case '*':
                //cout<<"test "<< leftOperand << root->mexp <<rightOperand<<endl;
                return ListenTCPServer::getMulDivData(leftOperand, rightOperand, '*');  // Call Server
            case '/':
                //cout<<"test "<< leftOperand << root->mexp <<rightOperand<<endl;
                return ListenTCPServer::getMulDivData(leftOperand, rightOperand, '/');  // Call Server
            default:
                cout << "What is This ?!?" << root->mexp << endl;
                exit(1);
        }
    }
};


int main() {
    auto test = make_unique<ListenTCPServer>();
    auto makeTreeModule = make_shared<MakeExpTree>();
    test->startListen(makeTreeModule);
    return 0;
}