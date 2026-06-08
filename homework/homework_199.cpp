#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

// Function to read CSV and store in 2D vector
vector<vector<double>> readCSV(const string &filename, size_t &rows, size_t &cols)
{
    ifstream file(filename);
    vector<vector<double>> data;
    string line;

    rows = 0;
    cols = 0;

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << endl;
        return data;
    }

    while (getline(file, line))
    {
        stringstream ss(line);
        string cell;

        vector<double> row;
        while (getline(ss, cell, ','))
        {
            row.push_back(stod(cell)); // convert string to double
        }

        if (cols == 0)
        {
            cols = row.size();
        }
        else if (row.size() != cols)
        {
            cerr << "Error: inconsistent number of columns in row " << rows << endl;
            return {};
        }

        data.push_back(row);
        rows++;
    }

    file.close();
    return data;
}

void minMaxScale(vector<vector<double>> &data)
{
    if (data.empty())
        return;

    size_t rows = data.size();
    size_t cols = data[0].size();

    for (size_t j = 0; j < cols; j++)
    {
        vector<double> column(rows);
        for (size_t i = 0; i < rows; i++)
            column[i] = data[i][j];

        auto [minIt, maxIt] = minmax_element(column.begin(), column.end());
        double minVal = *minIt;
        double maxVal = *maxIt;
        double range = maxVal - minVal;

        if (range != 0)
        {
            transform(column.begin(), column.end(), column.begin(),
                      [minVal, range](double x)
                      { return (x - minVal) / range; });
        }
        else
        {
            fill(column.begin(), column.end(), 0.0);
        }

        for (size_t i = 0; i < rows; i++)
            data[i][j] = column[i];
    }
}

void printData(const vector<vector<double>> &data)
{
    for (const auto &row : data)
    {
        for (size_t i = 0; i < row.size(); i++)
        {
            cout << row[i];
            if (i != row.size() - 1)
                cout << ", ";
        }
        cout << endl;
    }
}

int main()
{
    string filename = "data.csv"; // replace with your file
    size_t rows, cols;

    // Step 1: Read file
    vector<vector<double>> data = readCSV(filename, rows, cols);

    cout << "Rows: " << rows << ", Columns: " << cols << endl;

    // Step 2: Scale data
    minMaxScale(data);

    // Step 3: Print scaled data
    cout << "Scaled Data:" << endl;
    printData(data);

    return 0;
}