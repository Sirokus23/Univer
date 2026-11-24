import random

class Players:
    def __init__(self,name):
        self.name = name
        self.other_name = None
        self.score = 0
        self.history = []
        self.other_history = []

    def make_move(self):
        raise NotImplementedError

    def update(self, my_move, other_move, points):
        self.history.append(my_move)
        self.other_history.append(other_move)
        self.score += points

    def reset(self):
        self.score = 0
        self.history = []
        self.other_history = []

class RandomPlayer(Players):
    def make_move(self):
        return random.choice(['C', 'D'])

class DefectPlayer(Players):
    def make_move(self):
        return 'D'

class ProvizoriuEzWin(Players):
    def make_move(self):
        if not self.other_history:
            return 'C'
        elif self.other_history[-1] == 'C':
            return 'D'
        else:
            return 'D'

class TitForTatPlayer(Players):
    def make_move(self):
        if not self.other_history:
            return 'C'
        else:
            return self.other_history[-1]

class Game:
    REZULTATE = {
    ('C', 'C'): (3, 3),
    ('C', 'D'): (0, 5),
    ('D', 'C'): (5, 0),
    ('D', 'D'): (1, 1)
}
    def __init__(self, player1, player2, rounds=10):
        self.player1 = player1
        self.player2 = player2
        self.rounds = rounds

    def play(self):
        for _ in range(self.rounds):
            move1 = self.player1.make_move()
            move2 = self.player2.make_move()
            points1, points2 = self.REZULTATE[(move1, move2)]
            self.player1.update(move1, move2, points1)
            self.player2.update(move2, move1, points2)

    def print_results(self):
        print(f"{self.player1.name} score: {self.player1.score}")
        print(f"{self.player2.name} score: {self.player2.score}")
        print(f"{self.player1.name} history: {self.player1.history}")
        print(f"{self.player2.name} history: {self.player2.history}")

if __name__ == "__main__":
    player1 = TitForTatPlayer("Random Player")
    player2 = TitForTatPlayer("Tit For Tat Player")
    game = Game(player1, player2, rounds=100)
    game.play()
    game.print_results()