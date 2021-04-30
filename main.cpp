#include <csplines.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <math.h>

sf::VertexArray DrawCross(int size, float xPos, float yPos);
float GraphToPixel(float point, bool yValue, float pixelDensity, float offset, float windowDimension);
float PixelToGraph(float pixel, bool yValue, float pixelDensity, float offset, float windowDimension);

int main(){
    int windowX = 500;
    int windowY = 500;
    int graphOffset = 25;
    int scaleX = 10;
    int scaleY = 10;
    int pointSize = 10;
    float pixelDensityX = static_cast<float>(scaleX)/windowX;
    float pixelDensityY = static_cast<float>(scaleY)/windowY;
    const int workArea[2] = {(windowX - graphOffset), (windowY - graphOffset)}; 

    sf::VertexArray axes(sf::LinesStrip, 3);
    axes[0].position = sf::Vector2f(graphOffset, graphOffset);
    axes[1].position = sf::Vector2f(graphOffset, windowY - graphOffset);
    axes[2].position = sf::Vector2f(windowX - graphOffset, windowY - graphOffset);

    sf::RenderWindow window(sf::VideoMode(windowX,windowY), "Graph time!!!!");

    std::vector<point> points = {{1,2},{2,3},{3,5},{5,5},{9,0}};
    std::vector<coefficients> splines = Interpolate(points);
    for(size_t i = 0, l = splines.size(); i < l; i++){
        std::cout << splines[i].D << "x^3 + " << splines[i].C << "x^2 + " << splines[i].B << "x + " << splines[i].A << "\n";
        std::cout << "between points: " << points[i].x << "," << points[i].y << " and " << points[i+1].x << "," << points[i+1].y << "\n";
    }

    sf::VertexArray graph(sf::LineStrip,  workArea[0]);
    std::vector<sf::VertexArray> pointMarks;
    std::vector<sf::CircleShape> pointCircles;

    for(int i = 0; i < workArea[0]; i++){
        //i is the pixel being addressed
        //convert to numerical values
        float x = i*pixelDensityY;

        auto it = points.begin();
        while(it != points.end()){
            if(x == it->x){
                float pointX = GraphToPixel(it->x, false, pixelDensityY, graphOffset, windowX);
                float pointY = GraphToPixel(it->y, true, pixelDensityY, graphOffset, windowY);
                pointMarks.push_back(DrawCross(pointSize, pointX, pointY));

                sf::CircleShape circle;
                circle.setRadius(pointSize);
                circle.setPosition(pointX-pointSize, pointY-pointSize);
                circle.setOutlineThickness(1);
                circle.setFillColor(sf::Color::Black);
                circle.setOutlineColor(sf::Color::White);

                pointCircles.push_back(circle);
            }
            it++;
        }
        
        //set relation of y and x
        float y;
        for(size_t i = 0, l = splines.size(); i < l; i++){
            if(x <= points[i+1].x){
                float f = x - points[i].x;
                y = splines[i].D * pow(f,3) + splines[i].C * pow(f,2) + splines[i].B * f + splines[i].A;
                break;
            }
            else if(x > points.size()){
                float f = x - points[l-1].x;  
                y = splines[l-1].D * pow(f,3) + splines[l-1].C * pow(f,2) + splines[l-1].B * f + splines[l-1].A;
                break;
            }
        }

        x = GraphToPixel(x, false, pixelDensityX, graphOffset, windowX);
        y = GraphToPixel(y, true, pixelDensityY, graphOffset, windowY);
        
        if((y <= graphOffset)||(y >= workArea[0])){
            //std::cout << "out of graph bounds" << "\n";
            try {
                graph[i].position = graph[i-1].position;
            }
            catch(std::exception& e){
                graph[i].position = sf::Vector2f(graphOffset,graphOffset);
            }
            graph[i].color = sf::Color::Black;
        }

        graph[i].position = sf::Vector2f(x,y);
    }

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear(sf::Color::Black);
        //render sequence

        window.draw(axes);       
        for(size_t i = 0, l = pointMarks.size(); i < l; i++){
            window.draw(pointCircles[i]);
        }
        window.draw(graph);
        for(size_t i = 0, l = pointMarks.size(); i < l; i++){
            window.draw(pointMarks[i]);
        }

        window.display();
    }
    return 0;
}

sf::VertexArray DrawCross(int size, float xPos, float yPos){
    sf::VertexArray cross(sf::Lines, 4);
    size = size/2;
    cross[0] = sf::Vector2f(xPos + size, yPos + size);
    cross[1] = sf::Vector2f(xPos - size, yPos - size);
    cross[2] = sf::Vector2f(xPos + size, yPos - size);
    cross[3] = sf::Vector2f(xPos - size, yPos + size);

    cross[0].color = sf::Color::Magenta;
    cross[1].color = sf::Color::Magenta;
    cross[2].color = sf::Color::Magenta;
    cross[3].color = sf::Color::Magenta;

    return cross;
}

float GraphToPixel(float point, bool yValue, float pixelDensity, float offset, float windowDimension) {
    //convert back to pixel values
    float returnFloat = point/pixelDensity;

    //y pixels count from top of the screen, so reverse the direction
    //fit within the axes
    if(yValue){
        returnFloat = -returnFloat + windowDimension - offset;        
    }else{
        returnFloat += offset;
    }

    return returnFloat;
}

float PixelToGraph(float pixel, bool yValue, float pixelDensity, float offset, float windowDimension) {
    float returnFloat = pixel;

    if(yValue){
        returnFloat -= (windowDimension-offset);
    }else{
        returnFloat -= offset;
    }

    return returnFloat*pixelDensity;
}
