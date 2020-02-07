#include "telos_player.h"

void	TelosPlayer::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("test_method", "n"), &TelosPlayer::test_method);
	ClassDB::bind_method(D_METHOD("reset"), &TelosPlayer::reset);
	ClassDB::bind_method(D_METHOD("get_var"), &TelosPlayer::get_var);
}

int		TelosPlayer::get_var() const { return var; }
void	TelosPlayer::reset() { var = 0; }
void	TelosPlayer::test_method(int n) { var += n; }
TelosPlayer::TelosPlayer() {
	var = 0;
	std::cout << "TelosPlayer spawned\n";
}
