#include "Matrix.hpp"
#include <iostream>
#include <ctime>

Matrix::Matrix(int rows, int cols): rows(rows), cols(cols) {
	this->matrix = std::vector<int>(this->rows * this->cols);
}

void Matrix::initializeWithValues() {
	srand(time(nullptr));
	for (int i = 0; i < this->rows * this->cols; ++i) {
		this->matrix[i] = rand() % 10 + 1;
	}
}

int Matrix::getRows() const {
	return this->rows;
}

int Matrix::getCols() const {
	return this->cols;
}

int Matrix::getValue(int row, int col) {
	return this->matrix[static_cast<std::vector<int, std::allocator<int>>::size_type>(row) * this->cols + col];
}

void Matrix::setValue(int row, int col, int value) {
	this->matrix[static_cast<std::vector<int, std::allocator<int>>::size_type>(row) * this->cols + col] = value;
}

void Matrix::print() {
	for (int i = 0; i < this->rows; ++i) {
		for (int j = 0; j < this->cols; ++j) {
			std::cout << this->matrix[static_cast<std::vector<int, std::allocator<int>>::size_type>(i) * this->cols + j] << " ";
		}

		std::cout << std::endl;
	}
}
