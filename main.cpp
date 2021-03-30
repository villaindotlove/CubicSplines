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

int CheckSubset(std::vector<float> list1, std::vector<float> list2);

int FindFirstElement(std::vector<float> vector);

std::vector<float> MultiplyRowBy(std::vector<float> row, float value);

std::vector<float> ComplementRow(std::vector<float> target, std::vector<float> subset, int element);

int CountNonZero(std::vector<float> vector);

bool ValidSolution(std::vector<float> vector);

void VectorToSolution(std::vector<float> vector, std::vector<float>& solutions, std::vector<std::vector<float>>& matrix);
//

int main() {
    std::vector<point> points = {{1,2},{2,3},{3,5}};
    Interpolate(points);
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

    for(size_t i = 0, l = values.size(); i < l; i++){
            std::cout << values[i];
    }
    std::cout << std::endl;

    return returnCoeffs;   
}

std::vector<float> GaussianElimination(std::vector<std::vector<float>> matrix){
    std::vector<float> returnVals;
    std::vector<std::vector<float>> elimMatrix = matrix;
    
    //populate returnVals
    for(size_t i = 0, length = elimMatrix.size(); i < length; i++){
        returnVals.push_back(0);
    }
    //for each row i,
    for(size_t i = 0, length = elimMatrix.size(); i < length; i++){
        //in relation to every other row j,
        for(size_t j = 0; j < length; j++){
            if(i == j)
                continue;
            if(elimMatrix[i].empty()||elimMatrix[j].empty()){
                continue;
            }
            //if i is a subset of j or j is a subset of i
            int v = CheckSubset(elimMatrix[i], elimMatrix[j]);
            if(v == 0)
                continue;
            //subtract a multiple of the subset row such that one non zero value in the parent = 0;

            int first;
            if(v == 1) {//list i is a subset or they are equal sets
                first = FindFirstElement(elimMatrix[i]);
                elimMatrix[j] = ComplementRow(elimMatrix[j], elimMatrix[i], first);
            }
            if(v == 2) {//list j is a subset
                first = FindFirstElement(elimMatrix[j]);  
                elimMatrix[i] = ComplementRow(elimMatrix[i], elimMatrix[j], first);             
            }

            //if r.size() = 1 or 2 where r[length] != 0;
            if(ValidSolution(elimMatrix[i])){
                VectorToSolution(elimMatrix[i], returnVals, elimMatrix);            
            }
            else if(ValidSolution(elimMatrix[j])){
                VectorToSolution(elimMatrix[j], returnVals, elimMatrix);
            }
        }
    }
   
    return returnVals;
}

int CheckSubset(std::vector<float> list1, std::vector<float> list2){
    bool list1Subset = false;
    bool list2Subset = false;
    int returnVal;

    for(size_t i = 0, length = list1.size()-1; i < length; i++){
        if((list1[i] != 0)&&(list2[i] == 0)){
            if(list1Subset)
                return 0;
            else if(!list2Subset)
                list2Subset = true;
            else
                continue;
        }else if((list1[i] == 0)&&(list2[i] != 0)){
            if(list2Subset)
                return 0;
            else if(!list1Subset)
                list1Subset = true;
            else
                continue;
        }
    }

    if(list2Subset)
        return 2;
    else
        return 1;
}

int FindFirstElement(std::vector<float> vector){
    for(size_t i = 0, length = vector.size(); i < length; i++){
        if(vector[i] != 0){
            return i;
        }
    }
    return 0;
}

std::vector<float> MultiplyRowBy(std::vector<float> row, float value){
    std::vector<float> returnVec = {};
    for(size_t i = 0, length = row.size(); i < length; i++){
        returnVec.push_back(row[i] * value);
    }

    return returnVec;
}

std::vector<float> ComplementRow(std::vector<float> target, std::vector<float> subset, int element){
    float multiplier = target[element]/subset[element];
    std::vector<float> complementRow = MultiplyRowBy(subset, multiplier);
    std::vector<float> returnVec = {};
    
    for(size_t i = 0, length = target.size(); i < length; i++){
            returnVec.push_back(target[i] - complementRow[i]);
    }
    return returnVec;
}

int CountNonZero(std::vector<float> vector){
    int count = 0;
    for(size_t i = 0, length = vector.size(); i < length; i++){
        if(vector[i] != 0)
            count++;
    }
    return count;
}

bool ValidSolution(std::vector<float> vector){
    int n = CountNonZero(vector);
    if(n == 1){
        return true;
    }
    else if(n == 2){
        if(*vector.end() != 0){
            return true;
        }
    }else return false;
}

bool CheckEmpty(std::vector<float> vector){
    auto it = std::begin(vector);

    while(it != std::end(vector)){
        if(*it != 0){
            return false;
        }   
        else{
            it++;
        }
    }
    return true;

}

void VectorToSolution(std::vector<float> vector, std::vector<float>& solutions, std::vector<std::vector<float>>& matrix){
    //returnVals[i] = r[length]/r[i]
    int solutionIndex = FindFirstElement(vector);
    solutions[solutionIndex] = vector[vector.size()-1]/vector[solutionIndex];

    //for row in matrix
    for(size_t i = 0, length = matrix.size(); i < length; i++){
    //if row contains r[i]
        if(matrix[i][solutionIndex] != 0){
            //row[length] = row[i] * returnVals[i]
            //row[i] = 0
            matrix[i][length-1] -= matrix[i][solutionIndex] * solutions[solutionIndex];
            matrix[i][solutionIndex] = 0;
        }
    }
    
    //clean empty rows
    auto it = std::begin(matrix);

    while(it != std::end(matrix)){
        if(CheckEmpty(*it)){
            matrix.erase(it);
            continue;
        }
        it++;
    }
}
