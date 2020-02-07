#ifndef TELOS_PLAYER_H
# define TELOS_PLAYER_H

# include "core/reference.h"
# include <iostream>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <string.h>
# include <stdio.h>
# include <pthread.h>
# include <netdb.h>
# include <thread>
# include <errno.h>
# include <fstream>

typedef void * (*THREADCAST)(void*);

enum call_type {
	SYSTEM,
	USER
};

enum system_calls {
	HANDSHAKE
};

enum user_calls {
	MOVE,
	CHUNK
};

enum return_calls {
	HANDSHAKE_ACCEPT
};

class HyperNetManager : public Reference {
private:
	GDCLASS(HyperNetManager, Reference);
	void			*listener(void *d);
	int				_listener_status;
	char			_buffer[1024];
	int				_sock;
	std::thread		_t;
	std::string		_key;

protected:
	static void _bind_methods();

public:
	void		sendData(String s);
	void		setListenerStatus(int n);
	int			getListenerStatus() const;
	int			handhsake();
	std::string	genKey();
	HyperNetManager();
	~HyperNetManager();
};

#endif
