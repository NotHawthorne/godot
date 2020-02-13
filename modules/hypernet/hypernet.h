#ifndef TELOS_PLAYER_H
# define TELOS_PLAYER_H

extern "C" {
	# include "lua.h"
	# include "lualib.h"
	# include "lauxlib.h"
}

# include "core/reference.h"
# include "core/func_ref.h"
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
# include <map>
# include <sstream>
# include <vector>
# include "luabridge/Source/LuaBridge/LuaBridge.h"

typedef void * (*THREADCAST)(void*);
typedef int	(*lua_CFunction)(lua_State *L);

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
	GAME_STATE,
	REGISTER_UNIT,
	UPDATE_UNIT
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
	std::map<std::string, Ref<FuncRef>> _bindings;
	lua_State		*_L;
	std::thread		_luaThread;
	int				_luaStatus;
	std::string		_packetStack;
	unsigned		_ticks;

protected:
	static void _bind_methods();

public:
	void		sendData(String s);
	void		setListenerStatus(int n);
	int			getListenerStatus() const;
	void		sendPacket(int type, int subtype, std::string data);
	std::string	processPacket(std::string packet);
	std::string	genKey();
	int			moveUnit(std::string id, int dir);
	void		registerCallback(String s, Ref<FuncRef> r);
	void		runScript(String filepath);	
	void		*threadScript(void *data);
	void		addPacket(std::string packet);
	void		ticker(void);
	unsigned	timeSinceLastTick(void);
	std::string	structurePacket(int type, int subtype, std::string data);
	HyperNetManager();
	~HyperNetManager();
};

#endif
