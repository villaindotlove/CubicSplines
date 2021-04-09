#include <csplines.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <math.h>

int main(){
    int windowX = 800;
    int windowY = 600;
    int graphOffset = 15;
    const int workArea[2] = {windowX - graphOffset, windowY - graphOffset}; 
    int graphLimits = 10;

    sf::VertexArray axes(sf::LinesStrip, 3);
    axes[0].position = sf::Vector2f(graphOffset, graphOffset);
    axes[1].position = sf::Vector2f(graphOffset, windowY - graphOffset);
    axes[2].position = sf::Vector2f(windowX - graphOffset, windowY - graphOffset);

    sf::RenderWindow window(sf::VideoMode(windowX,windowY), "Graph time!!!!");

    std::vector<point> points = {{1,2},{2,3},{3,5}};
    std::vector<coefficients> c = Interpolate(points);
    for(size_t i = 0, l = c.size(); i < l; i++){
        std::cout << c[i].D << "x^3 + " << c[i].C << "x^2 + " << c[i].B << "x + " << c[i].A << "\n";
        std::cout << "between points: " << points[i].x << "," << points[i].y << " and " << points[i+1].x << "," << points[i+1].y << "\n";
    }

    sf::VertexArray graph(sf::LineStrip,  workArea[0]);

    for(int i = 0; i < workArea[0]; i++){
        float x = i;
        //float y = -((c[0].D * pow(i,3) + c[0].C * pow(i,2) + c[0].B * i + c[0].A))+(windowY-10);
        float y = -x+(windowY-graphOffset);

        if((y < graphOffset)||(y > workArea[0])){
            try {
                graph[i].position = graph[i-1].position;
            }
            catch(std::exception& e){
                graph[i].position = sf::Vector2f(graphOffset,graphOffset);
            }
            graph[i].color = sf::Color::Black;
        }
        std::cout << x << "," << y << "\n";

        graph[i].position = sf::Vector2f(x+graphOffset,y);
    }

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Black);
        window.draw(graph);
        window.draw(axes);
        window.display();
    }


    return 0;
}
