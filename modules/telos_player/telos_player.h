#ifndef TELOS_PLAYER_H
# define TELOS_PLAYER_H

# include "core/reference.h"
# include <iostream>

class TelosPlayer : public Reference {
private:
	GDCLASS(TelosPlayer, Reference);
	int	var;

protected:
	static void _bind_methods();

public:
	void	test_method(int n = 0);
	void	reset();
	int		get_var() const;
	TelosPlayer();
};

#endif
