#ifndef __MATH_H__
#define __MATH_H__

/**
 * @brief Computes multiplication of two ints
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-292]
 * @param x num1
 * @param y num2
 * @return int product
 */
int __mulsi3(int x, int y);


/**
 * @brief Computes division of two ints
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-292]
 * @param x dividend
 * @param y divisor
 * @return int quotient
 */
int __divsi3(int x, int y);


/**
 * @brief Computes remainder of division of two ints
 * 
 * @param x dividend
 * @param y divisor
 * @return int remainder
 */
int __modsi3(int x, int y);


/**
 * @brief Computes square root of given number
 * @see http://f.javier.io/rep/books/The%20Elements%20of%20Computing%20Systems.pdf [pg-293]
 * @param x num
 * @return int sqrt
 */
int sqrt(int x);


/**
 * @brief Returns |x|
 * 
 * @param x int
 * @return int absolute value
 */
int abs(int x);


/**
 * @brief Returns lesser of two ints (if equal, returns num1)
 * 
 * @param x num1
 * @param y num2
 * @return int min(num1, num2)
 */
int min(int x, int y);


/**
 * @brief Returns greater of two ints (if euqal, returns num1)
 * 
 * @param x num1
 * @param y num2
 * @return int max(num1, num2)
 */
int max(int x, int y);


#endif //__MATH_H__
