#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

struct Point3D {
    float x, y, z;
};

Point3D rotateX(const Point3D& p, float angle) { //calculations made with gemini
    float rad = angle * 3.14159265f / 180.0f;
    return {
        p.x,
        p.y * cosf(rad) - p.z * sinf(rad),
        p.y * sinf(rad) + p.z * cosf(rad)
    };
}

Point3D rotateY(const Point3D& p, float angle) { //calculation made with gemini
    float rad = angle * 3.14159265f / 180.0f;
    return {
        p.x * cosf(rad) + p.z * sinf(rad),
        p.y,
        -p.x * sinf(rad) + p.z * cosf(rad)
    };
}

struct Point2D {
    float x, y;
};

// Converts a 3D coordinate (X, Y, Z) into a 2D screen coordinate (X, Y)
Point2D convert(Point3D p, float originX, float originY, float scale = 60.0f) { //calculations made with gemini
    const float zAngle = 1.0f;
    const float zScale = 0.6f;

    float projX = p.x + (p.z * cosf(zAngle) * zScale);
    float projY = p.y + (p.z * sinf(zAngle) * zScale);

    return {
        originX + (projX * scale),
        originY - (projY * scale) //inverts because y is down
    };
}

struct Edge { //use edges to repeat points without creating duplicates
    std::size_t point1, point2;
};

struct Shape { //allows shape logic to be reused
	std::vector<Point3D> vertices;
    std::vector<Edge> edges;
};

Shape cube() {
    return {
        { {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
          {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1} },
        { 
          {0,1}, {1,2}, {2,3}, {3,0},
          {4,5}, {5,6}, {6,7}, {7,4},
          {0,4}, {1,5}, {2,6}, {3,7},
          {0,2}, {1,3}
        }
    };
}

Shape pyramid() {
    return {
        { {-1,-1,-1}, {1,-1,-1}, {1,-1,1}, {-1,-1,1}, {0,1,0} },
        { {0,1},{1,2},{2,3},{3,0}, {0,4},{1,4},{2,4},{3,4} }
    };
}

Shape tetrahedron() {
    return {
        { {0, 1.2f, 0}, {-1, -0.8f, -1}, {1, -0.8f, -1}, {0, -0.8f, 1} },
        { {0,1}, {0,2}, {0,3}, {1,2}, {2,3}, {3,1} }
    };
}

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "3D-floating-shape"); //used gemini to figure out window, gui and shapes
    const sf::Time frameTargetTime = sf::seconds(1.0f / 60.0f);
    sf::Clock frameClock;

    float originX = 400.0f;
    float originY = 300.0f;
    float cam_angle = 0.0f;
    float x_angle = 0.0f;
    float y_angle = 0.0f;
    vector<sf::Color> colors = {sf::Color::Red, sf::Color::Blue, sf::Color::Green};
    int coloridx = 0;
    bool show_axis = true; 

    vector<Shape> shapes = { cube(), pyramid(), tetrahedron() };
    int shapes_idx = 0;

    while (window.isOpen()) { //opening window 
		window.clear(sf::Color::White);
		frameClock.restart();
		coloridx = 0; //resets idx so it doesn't change colors each frame
		
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::S) {
                    shapes_idx = (shapes_idx + 1) % shapes.size();
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
					cam_angle += 0.01f;
					originX -= 20.0f;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
					cam_angle -= 0.01f;
					originX += 20.0f;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)) { //used to make testing faster
					window.close();
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) {
					if (show_axis) {
						show_axis = false;
					}
					else {
						show_axis = true;
					}
				}
            }
        }
        
        x_angle += 0.75;
        y_angle += 0.75f;
        const float len = 6.0f;

        Point2D pX_neg = convert({-len,  -cam_angle,  -cam_angle}, originX, originY);
        Point2D pX_pos = convert({ len,  cam_angle,  cam_angle}, originX, originY);

        Point2D pY_neg = convert({ 0.0f, -len + 0.5f,  0.0f}, originX, originY);
        Point2D pY_pos = convert({ 0.0f,  len - 0.5f,  0.0f}, originX, originY);

        Point2D pZ_neg = convert({ 0.0f,  0.0f, -5.0}, originX, originY);
        Point2D pZ_pos = convert({ 0.0f,  0.0f,  2.5f}, originX, originY);

        sf::Vertex axisLines[] = {

            sf::Vertex{sf::Vector2f(pX_neg.x, pX_neg.y), sf::Color::Black},
            sf::Vertex{sf::Vector2f(pX_pos.x, pX_pos.y), sf::Color::Black},

            sf::Vertex{sf::Vector2f(pY_neg.x, pY_neg.y), sf::Color::Black},
            sf::Vertex{sf::Vector2f(pY_pos.x, pY_pos.y), sf::Color::Black},

            sf::Vertex{sf::Vector2f(pZ_neg.x, pZ_neg.y), sf::Color::Black},
            sf::Vertex{sf::Vector2f(pZ_pos.x, pZ_pos.y), sf::Color::Black}
        };
		if (show_axis) {
			window.draw(axisLines, 6, sf::PrimitiveType::Lines);
		}
        
		const Shape& currentShape = shapes[shapes_idx];
        vector<Point2D> transformed2D;

        for (const auto& vertex : currentShape.vertices) {
            Point3D rotX = rotateX(vertex, x_angle);
			Point3D rotXY = rotateY(rotX, y_angle);
			Point2D projected = convert(rotXY, originX, originY);
			transformed2D.push_back(projected);
        }

        for (const auto& edge : currentShape.edges) {
            Point2D p1 = transformed2D[edge.point1];
            Point2D p2 = transformed2D[edge.point2];

            sf::Vertex line[] = {
                sf::Vertex{sf::Vector2f(p1.x, p1.y), colors[coloridx]},
                sf::Vertex{sf::Vector2f(p2.x, p2.y), colors[coloridx]}
            };
            window.draw(line, 2, sf::PrimitiveType::Lines);
            coloridx = (coloridx + 1) % 3;
        }

		window.display();

        sf::Time elapsedTime = frameClock.getElapsedTime();
        sf::Time sleepTime = frameTargetTime - elapsedTime;
        //cout << "slept for " << sleepTime.asMilliseconds() << endl; //used for testing
        if (sleepTime > sf::Time::Zero) {
            sf::sleep(sleepTime);
        }
    }

    return 0;
}
