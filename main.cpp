#include <iostream>
#include <vector>
#include <cmath>

struct point {
    int x;
    int y;
};

struct coefficients {
    float A;
    float B;
    float C;
    float D;
};

std::vector<coefficients> Interpolate(std::vector<point> points);

std::vector<float> GaussianElimination(std::vector<std::vector<float>> matrix);

int FindFirstElement(std::vector<float> vector);

std::vector<float> MultiplyRowBy(std::vector<float> row, float value);

std::vector<float> ComplementRow(std::vector<float> target, std::vector<float> complement, int element);

// void VectorToSolution(std::vector<float> vector, std::vector<float>& solutions, std::vector<std::vector<float>>& matrix);
//

int main() {
    std::vector<point> points = {{1,2},{2,3},{3,5}};
    std::vector<coefficients> c = Interpolate(points);
    for(size_t i = 0, l = c.size(); i < l; i++){
        std::cout << c[i].D << "x^3 + " << c[i].C << "x^2 + " << c[i].B << "x + " << c[i].A << "\n";
        std::cout << "between points: " << points[i].x << "," << points[i].y << " and " << points[i+1].x << "," << points[i+1].y << "\n";
    }
    return 0;
}

//

std::vector<coefficients> Interpolate(std::vector<point> points){

    std::vector<coefficients> returnCoeffs;
    std::vector<std::vector<float>> equationMatrix;
    
    int splines = points.size() - 1;
    int pairs = splines - 1;
    int row_id = 0;

    //Fill empty equation matrix
    for(int i = 0, end = 3*splines; i < end; i++)
    {
        std::vector<float> empty;
        for (int j = 0; j <= end; j++) //extra slot for known values
            empty.push_back(0);
        equationMatrix.push_back(empty);
    }

    //Common points match
    for(int i = 0, end = splines; i < end; i++)
    {
        returnCoeffs.push_back({0,0,0,0});

        returnCoeffs[i].A = points[i].y;
        int lwr = points[i].x;
        int upr = points[i+1].x;

        int matrix_i = i*3;

        //for each point[] from 0->(n-1)
        //Sn(x) = An + Bn(x-xn) + Cn(x-xn)^2 + Dn(x-xn)^3

        //for point[0]: (1,2)
        //S0(x) = A0 + B0(x-1) + C0(x-1)^2 + D0(x-1)^3
        //A0 = S0(x0)

        equationMatrix[i][matrix_i] = (upr-lwr);
        equationMatrix[i][matrix_i+1] = pow(upr-lwr, 2);
        equationMatrix[i][matrix_i+2] = pow(upr-lwr, 3);
        equationMatrix[i].back() = points[i+1].y - points[i].y;
    }
    row_id = splines;

    //At the point of intersection between splines s0 and s1
    //S'n(x) = S'n+1(x)
    //and
    //S''n(x) = S''n+1(x)
    //
    //S'n(x) = Bn + 2Cn(x - xn) + 3Dn(x - xn)^2
    //S''n(x) = 2Cn + 6Dn(x - xn)

    //First and second differential match
    for(int i = 0; i < pairs; i++)
    {  
        int matrix_i = 3*i;

        equationMatrix[row_id][matrix_i] = 1;
        equationMatrix[row_id][matrix_i+1] = 2*(points[i+1].x - points[i].x);
        equationMatrix[row_id][matrix_i+2] = 3*pow(points[i+1].x - points[i].x, 2);
        equationMatrix[row_id][matrix_i+3] = -1;

        equationMatrix[row_id+1][matrix_i+1] = 1;
        equationMatrix[row_id+1][matrix_i+2] = 3*(points[i+1].x - points[i].x);
        equationMatrix[row_id+1][matrix_i+4] = -1;

        row_id += 2;
    }

    //Boundary Condition
    //At both boundaries of the line (assuming natural bounding)
    //S''n(x) = 0

    //Lower bound 2Cn + 6Dn(x - xn) = 0

    equationMatrix[row_id][1] = 2;

    row_id++;


    //Upper bound S''n(xn) = 0

    int row = equationMatrix[row_id].size()-1;
    int size_p = points.size() - 1;

    equationMatrix[row_id][row-2] = 2;
    equationMatrix[row_id][row-1] = 6 * (points[size_p].x - points[size_p-1].x);

    //Gaussian Elimination

    std::vector<float> values = GaussianElimination(equationMatrix);

    for(size_t i = 0, l = values.size()/3; i < l; i++){
        size_t seg = 3*i;
        returnCoeffs[i].B = values[seg];
        returnCoeffs[i].C = values[seg+1];
        returnCoeffs[i].D = values[seg+2];
    }
    
    return returnCoeffs;   
}

std::vector<float> GaussianElimination(std::vector<std::vector<float>> matrix){
    std::vector<float> returnVals;
    std::vector<std::vector<float>> elimMatrix = matrix;
    std::vector<std::vector<float>> rowEchelonForm;
    //populate returnVals
    for(size_t i = 0, length = elimMatrix.size(); i < length; i++){
        returnVals.push_back(0);
    }

    for(size_t i = 0, length = elimMatrix.size()-1; i < length; i++){

        std::vector<std::vector<float>*> p_conflicts = {};
        std::vector<std::vector<float>>::iterator leadingCoefficient;

        auto it = std::begin(elimMatrix);

        while(it != std::end(elimMatrix)){
            std::vector<float> row = *it; 
            if(row[i] != 0){
                std::vector<float>* p = &*it;
                if(p_conflicts.empty()){
                    leadingCoefficient = it;
                }
                p_conflicts.push_back(p);
            }     
            it++;
        }

        if(p_conflicts.size() > 1){
            std::vector<float>* main = p_conflicts[0];
            
            auto it = std::begin(p_conflicts);
            it++;

            while(it != std::end(p_conflicts)){
                **it = ComplementRow(**it, *main, i);
                it++;
            }
            rowEchelonForm.push_back(*main);
            elimMatrix.erase(leadingCoefficient);
        }
    }
    rowEchelonForm.push_back(elimMatrix[0]);

    auto it = std::rbegin(rowEchelonForm);

    while(it != std::rend(rowEchelonForm)){
        //assume that *it will be in solution form.
        int element = FindFirstElement(*it);
        float known = (*it).back();
        float unknown = (*it)[element];
        float val = known/unknown;
        returnVals[element] = val;

        auto it_c = it;
        while(it_c != std::rend(rowEchelonForm)){
            (*it_c)[(*it_c).size()-1] -= (*it_c)[element] * val;
            (*it_c)[element] = 0;
            it_c++;
        }
        it++;
    }
    return returnVals;
}

int FindFirstElement(std::vector<float> vector){
    for(size_t i = 0, length = vector.size(); i < length; i++){
        if(vector[i] != 0){
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::vector<float> MultiplyRowBy(std::vector<float> row, float value){
    std::vector<float> returnVec = {};
    for(size_t i = 0, length = row.size(); i < length; i++){
        returnVec.push_back(row[i] * value);
    }

    return returnVec;
}

std::vector<float> ComplementRow(std::vector<float> target, std::vector<float> complement, int element){
    float multiplier = target[element]/complement[element];
    std::vector<float> complementRow = MultiplyRowBy(complement, multiplier);
    std::vector<float> returnVec = {};
    
    for(size_t i = 0, length = target.size(); i < length; i++){
            returnVec.push_back(target[i] - complementRow[i]);
    }
    return returnVec;
}
