#include <csplines.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <math.h>

sf::VertexArray DrawCross(int size, float xPos, float yPos);
sf::CircleShape DrawCircle(int size, int xPos, int yPos);
int GraphToPixel(float point, bool yValue, float pixelDensity, float offset, float windowDimension);
float PixelToGraph(float pixel, bool yValue, float pixelDensity, float offset, float windowDimension);

int main(){

    sf::Font font;
    if (!font.loadFromFile("./res/LiberationSansNarrow-Regular.ttf"))
    {
        std::cout << "Failed to load font from file." << "\n";
        return 1;
    }

    const int windowX = 500;
    const int windowY = 500;
    const int graphOffset = 25;
    const int scaleX = 20;
    const int scaleY = 20;
    const int pointSize = 10;
    const float pixelDensityX = static_cast<float>(scaleX)/windowX;
    const float pixelDensityY = static_cast<float>(scaleY)/windowY;
    const int workArea[2] = {(windowX - graphOffset), (windowY - graphOffset)}; 

    sf::RenderWindow window(sf::VideoMode(windowX,windowY), "Graph time!!!!");

    std::vector<point> graphPoints = {{0,0},{1,2},{2,3},{3,5},{5,5},{7,7},{9,0}};
    std::vector<coefficients> splines = Interpolate(graphPoints);
    for(size_t i = 0, l = splines.size(); i < l; i++){
        std::cout << splines[i].D << "x^3 + " << splines[i].C << "x^2 + " << splines[i].B << "x + " << splines[i].A << "\n";
        std::cout << "between points: " << graphPoints[i].x << "," << graphPoints[i].y << " and " << graphPoints[i+1].x << "," << graphPoints[i+1].y << "\n";
    }

    sf::VertexArray axes(sf::LinesStrip, 3);
    axes[0].position = sf::Vector2f(graphOffset, graphOffset);
    axes[1].position = sf::Vector2f(graphOffset, windowY - graphOffset);
    axes[2].position = sf::Vector2f(windowX - graphOffset, windowY - graphOffset);

    sf::Text axesLabels[2];
    axesLabels[0].setFont(font);
    axesLabels[0].setString("Y");
    axesLabels[0].setPosition(graphOffset/4, graphOffset);
    axesLabels[0].setCharacterSize(18);
    axesLabels[1].setFont(font);
    axesLabels[1].setString("X");
    axesLabels[1].setPosition(windowX-2*graphOffset, windowY-graphOffset);
    axesLabels[1].setCharacterSize(18);

    sf::VertexArray graph(sf::LineStrip,  workArea[0]);
    std::vector<sf::VertexArray> pointMarks;
    std::vector<sf::CircleShape> pointCircles;
    std::vector<sf::Text>        pointLabels;
    std::vector<point>           pointCoordinates;

    //create points
    auto it = graphPoints.begin();
    while(it != graphPoints.end()){
        int pointX = GraphToPixel(it->x, false, pixelDensityX, graphOffset, windowX);
        int pointY = GraphToPixel(it->y, true, pixelDensityY, graphOffset, windowY);

        pointMarks.push_back(DrawCross(pointSize, pointX, pointY));

        sf::String label = "(" + std::to_string(it->x) + "," + std::to_string(it->y) + ")";
        sf::Text t = sf::Text(label,font,18);
        t.setColor(sf::Color::Green);
        t.setPosition(pointX+(graphOffset/2),pointY-graphOffset);
        pointLabels.push_back(t);
        
        sf::CircleShape circle = DrawCircle(pointSize, pointX, pointY);

        pointCircles.push_back(circle);
        
        point p = {pointX,pointY};
        pointCoordinates.push_back(p);
        it++;
    }
        

    //fill graph with vertices
    for(int i = 0; i < workArea[0]; i++){
        //i is the pixel being addressed
        //convert to numerical values
        float x = i*pixelDensityY;
        //set relation of y and x
        float y;
        for(size_t i = 0, l = splines.size(); i < l; i++){
            float f;
            if(x <= graphPoints[i+1].x){
                f = x - graphPoints[i].x;
                y = splines[i].D * pow(f,3) + splines[i].C * pow(f,2) + splines[i].B * f + splines[i].A;
                break;
            }
            else if(x > graphPoints.size()){
                f = x - graphPoints[l-1].x;  
                y = splines[l-1].D * pow(f,3) + splines[l-1].C * pow(f,2) + splines[l-1].B * f + splines[l-1].A;
                break;
            }
        }

        x = GraphToPixel(x, false, pixelDensityX, graphOffset, windowX);
        y = GraphToPixel(y, true, pixelDensityY, graphOffset, windowY);
        
        //test for points being out of bounds
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
    
    int selected = -1;

    while(window.isOpen()){
        sf::Event event;

        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        
        if(window.hasFocus()){
            //interaction begin
            // std::vector<sf::VertexArray> pointMarks;
            // std::vector<sf::CircleShape> pointCircles;
            // std::vector<sf::Text>        pointLabels;
            // std::vector<point>           pointCoordinates;
            sf::Vector2i position = sf::Mouse::getPosition(window);
            if(selected != -1){
                if(!sf::Mouse::isButtonPressed(sf::Mouse::Left)){
                    selected = -1;
                }else{
                    pointMarks[selected] = DrawCross(pointSize, position.x, position.y);
                    pointCircles[selected] = DrawCircle(pointSize, position.x, position.y);
                }
            }
            else if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                for(int i = 0, l = pointCoordinates.size(); i < l; i++){
                    //compare mouse coordinates with point locations
                    int x = pointCoordinates[i].x;
                    int y = pointCoordinates[i].y;
                    if((position.x < x+pointSize)&&(position.x > x-pointSize)){
                        if((position.y < y+pointSize)&&(position.y > y-pointSize)){
                            //point clicked on
                            selected = i;
                        }
                    }
                }
            }
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
            window.draw(pointLabels[i]);
        }
        window.draw(axesLabels[0]);
        window.draw(axesLabels[1]);
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

sf::CircleShape DrawCircle(int size, int xPos, int yPos){
    sf::CircleShape circle;
    circle.setRadius(size);
    circle.setPosition(xPos-size, yPos-size);
    circle.setOutlineThickness(1);
    circle.setFillColor(sf::Color::Black);
    circle.setOutlineColor(sf::Color::White);

    return circle;
}

int GraphToPixel(float point, bool yValue, float pixelDensity, float offset, float windowDimension) {
    //convert back to pixel values
    float returnInt = point/pixelDensity;

    //y pixels count from top of the screen, so reverse the direction
    //fit within the axes
    if(yValue){
        returnInt = -returnInt + windowDimension - offset;        
    }else{
        returnInt += offset;
    }

    return returnInt;
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
