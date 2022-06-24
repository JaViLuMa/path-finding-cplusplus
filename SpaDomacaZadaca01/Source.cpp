#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>

constexpr auto WIDTH = 1000;
constexpr auto HEIGHT = 1000;

constexpr auto BOARD_COLUMNS = 50;
constexpr auto BOARD_ROWS = BOARD_COLUMNS;
constexpr auto WALL_THRESHOLD = 0.1f;

constexpr auto FPS = 60;
constexpr auto TITLE = "Matej Ruzic, 1IP1";

using namespace std;
using namespace sf;


struct Node
{
	Node(int i = 0, int j = 0) { this->i = i; this->j = j; }

	int i, j;

	float f = 0,
		g = 0,
		h = 0;

	bool wall = false;

	vector<Node*> neighbors;

	Node* previous = nullptr;
	// Node* current = nullptr;

	void addNeighbors(vector<vector<Node*>>& g)
	{
		this->i = i;
		this->j = j;

		if (i < BOARD_COLUMNS - 1) this->neighbors.push_back(g[i + 1][j]);
		if (i > 0) this->neighbors.push_back(g[i - 1][j]);
		if (j < BOARD_ROWS - 1) this->neighbors.push_back(g[i][j + 1]);
		if (j > 0) this->neighbors.push_back(g[i][j - 1]);
	}


	RectangleShape draw(int i, int j, const Color &color)
	{
		Vector2f cellSize(ceil(HEIGHT / BOARD_ROWS), ceil(WIDTH / BOARD_COLUMNS));

		RectangleShape cell;
		cell.setPosition(i * cellSize.x, j * cellSize.y);
		cell.setSize(cellSize);
		cell.setFillColor(color);

		return cell;
	}


	void show(RenderWindow &w, const Color &color)
	{
		if (this->wall) w.draw(draw(this->i, this->j, Color::Black));
		else w.draw(draw(this->i, this->j, color));
	}


	bool operator==(const Node*& n) const
	{
		return i == n->i && j == n->j;
	}
};


double heuristics(Node*& n, Node*& ep)
{
	return abs(n->i - ep->i) + abs(n->j - ep->j);
}


void popOpenSet(vector<Node*>& os, Node*& c)
{
	for (int i = os.size() - 1; i >= 0; i--)
	{
		if (os[i] == c) os.erase(os.begin() + i);
	}
}


RectangleShape makeCell(int i, int j, bool wall, Vector2f& cellSize)
{
	RectangleShape cell;
	cell.setPosition(i * cellSize.x, j * cellSize.y);
	cell.setSize(cellSize);
	cell.setFillColor(wall ? Color::Black : Color::White);

	if (i == 0 && j == 0) cell.setFillColor(Color::Yellow);
	if (i == BOARD_ROWS - 1 && j == BOARD_COLUMNS - 1) cell.setFillColor(Color::Green);

	return cell;
}


void showCells(RenderWindow& window, vector<vector<Node*>>& g)
{
	Vector2f cellSize(ceil(HEIGHT / BOARD_ROWS), ceil(WIDTH / BOARD_COLUMNS));

	for (int row = 0; row < BOARD_ROWS; row++)
	{
		for (int column = 0; column < BOARD_COLUMNS; column++)
		{
			window.draw(makeCell(row, column, g[row][column]->wall, cellSize));
		}
	}
}


void addNeighbors(vector<vector<Node*>> &g)
{
	for (int row = 0; row < BOARD_ROWS; row++)
	{
		for (int column = 0; column < BOARD_COLUMNS; column++)
		{
			g[row][column]->addNeighbors(g);
		}
	}
}


void prepareGrid(vector<vector<double>>& tg, vector<vector<Node*>>& g)
{
	double random = 0.0;


	for (int row = 0; row < BOARD_ROWS; row++)
	{
		for (int column = 0; column < BOARD_COLUMNS; column++)
		{
			g[row][column] = new Node(row, column);
			g[row][column]->wall = tg[row][column];
		}
	}
}


void prepareTemplateGrid(vector<vector<double>>& tg)
{
	double random = 0.0;

	for (int row = 0; row < BOARD_ROWS; row++)
	{
		for (int column = 0; column < BOARD_COLUMNS; column++)
		{
			random = (double)(rand()) / (RAND_MAX);

			if ((row == 0 && column == 0) || (row == BOARD_ROWS - 1 && column == BOARD_COLUMNS - 1)) tg[row][column] = 0;
			else
			{
				random < WALL_THRESHOLD ? tg[row][column] = 1 : tg[row][column] = 0;
			}
		}
	}
}


RectangleShape drawPathNode(Node*& n)
{
	Vector2f cellSize(ceil(HEIGHT / BOARD_ROWS), ceil(WIDTH / BOARD_COLUMNS));

	RectangleShape cell;
	cell.setPosition(n->i * cellSize.x, n->j * cellSize.y);
	cell.setSize(cellSize);
	cell.setFillColor(Color::Color(50, 0, 255));

	return cell;
}


RectangleShape drawClosedSetNodes(Node*& n)
{
	Vector2f cellSize(ceil(HEIGHT / BOARD_ROWS), ceil(WIDTH / BOARD_COLUMNS));

	RectangleShape cell;
	cell.setPosition(n->i * cellSize.x, n->j * cellSize.y);
	cell.setSize(cellSize);
	cell.setFillColor(Color::Color(150, 0, 0, 100));

	return cell;
}


RectangleShape drawOpenSetNodes(Node*& n)
{
	Vector2f cellSize(ceil(HEIGHT / BOARD_ROWS), ceil(WIDTH / BOARD_COLUMNS));

	RectangleShape cell;
	cell.setPosition(n->i * cellSize.x, n->j * cellSize.y);
	cell.setSize(cellSize);
	cell.setFillColor(Color::Color(0, 150, 0, 100));

	return cell;
}


int main()
{
	srand(time(nullptr));

	RenderWindow window(VideoMode(WIDTH, HEIGHT), TITLE);
	window.setFramerateLimit(FPS);

	vector<Node*> openSet;
	vector<Node*> closedSet;

	vector<vector<double>> templateGrid;
	vector<vector<Node*>> grid;

	templateGrid.resize(BOARD_ROWS, vector<double>(BOARD_COLUMNS));
	grid.resize(BOARD_ROWS, vector<Node*>(BOARD_COLUMNS));

	prepareTemplateGrid(templateGrid);
	prepareGrid(templateGrid, grid);
	addNeighbors(grid);

	Node* startingPosition = grid[0][0];
	Node* endingPosition = grid[BOARD_ROWS - 1][BOARD_COLUMNS - 1];

	openSet.push_back(startingPosition);

	Node *current{};

	while (window.isOpen())
	{
		Event event;

		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed) window.close();
		}

		window.clear(Color::White);

		if (openSet.size() > 0)
		{
			int lowIndex = 0;

			for (int i = 0; i < openSet.size(); i++)
			{
				if (openSet[i]->f < openSet[lowIndex]->f)
				{
					lowIndex = i;
				}
			}

			current = openSet[lowIndex];

			if (current == endingPosition)
			{
				cout << "Path Finding COMPLETE!!!" << endl;
				break;
			}

			popOpenSet(openSet, current);
			closedSet.push_back(current);

			vector<Node*> neighbors = current->neighbors;

			for (int i = 0; i < neighbors.size(); i++)
			{
				Node* neighbor = neighbors[i];

				vector<Node*>::iterator closedSetIt;
				closedSetIt = find(closedSet.begin(), closedSet.end(), neighbor);

				if (closedSetIt == closedSet.end() && neighbor->wall == false)
				{
					double tempG = neighbor->g + 1;

					bool newPath = false;

					vector<Node*>::iterator openSetIt;
					openSetIt = find(openSet.begin(), openSet.end(), neighbor);

					if (openSetIt != openSet.end())
					{
						if (tempG < neighbor->g)
						{
							neighbor->g = tempG;
							newPath = true;
						}
					}

					else
					{
						neighbor->g = tempG;
						newPath = true;
						openSet.push_back(neighbor);
					}

					if (newPath)
					{
						neighbor->h = heuristics(neighbor, endingPosition);
						neighbor->f = neighbor->g + neighbor->h;
						neighbor->previous = current;
					}
				}
			}
		}

		else
		{
			cout << "NO SOLUTION" << endl;
			break;
		}

		for (int row = 0; row < BOARD_ROWS; row++)
		{
			for (int column = 0; column < BOARD_COLUMNS; column++)
			{
				grid[row][column]->show(window, Color::White);
			}
		}

		for (int i = 0; i < closedSet.size(); i++)
		{
			int ii = closedSet[i]->i;
			int jj = closedSet[i]->j;

			closedSet[i]->show(window, Color::Color(150, 0, 0, 100));
		}

		for (int i = 0; i < openSet.size(); i++)
		{
			int ii = openSet[i]->i;
			int jj = openSet[i]->j;

			openSet[i]->show(window, Color::Color(0, 150, 0, 100));
		}

		vector<Node*> path;

		Node* temp = current;

		path.push_back(current);

		while (temp->previous)
		{
			path.push_back(temp->previous);
			temp = temp->previous;
		}

		for (int i = 0; i < path.size(); i++)
		{
			path[i]->show(window, Color::Color(50, 0, 255));
			grid[0][0]->show(window, Color::Green);
			grid[BOARD_ROWS - 1][BOARD_COLUMNS - 1]->show(window, Color::Yellow);
		}

		window.display();
	}

	return 0;
}
