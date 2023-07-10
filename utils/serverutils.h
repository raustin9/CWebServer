#pragma once
#ifndef _SERVERUTILS
#define _SERVERUTILS


// Structure for the server
struct Server { 
  int Port;    // port the server will listen on
  int Backlog; // the backlog of connections for the queue
};

#endif // _SERVERUTILS
