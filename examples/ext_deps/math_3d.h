/**

Math 3D v1.0
By Stephan Soller <stephan.soller@helionweb.de> and Tobias Malmsheimer
Licensed under the MIT license

Math 3D is a compact C99 library meant to be used with OpenGL. It provides basic
3D vector and 4x4 matrix operations as well as functions to create transformation
and projection matrices. The OpenGL binary layout is used so you can just upload
vectors and matrices into shaders and work with them without any conversions.

It's an stb style single header file library. Define MATH_3D_IMPLEMENTATION
before you include this file in *one* C file to create the implementation.


QUICK NOTES

- If not explicitly stated by a parameter name all angles are in radians.
- The matrices use column-major indices. This is the same as in OpenGL and GLSL.
  The matrix documentation below for details.
- Matrices are passed by value. This is probably a bit inefficient but
  simplifies code quite a bit. Most operations will be inlined by the compiler
  anyway so the difference shouldn't matter that much. A matrix fits into 4 of
  the 16 SSE2 registers anyway. If profiling shows significant slowdowns the
  matrix type might change but ease of use is more important than every last
  percent of performance.
- When combining matrices with multiplication the effects apply right to left.
  This is the convention used in mathematics and OpenGL. Source:
  https://en.wikipedia.org/wiki/Transformation_matrix#Composing_and_inverting_transformations
  Direct3D does it differently.
- The `m4_mul_pos()` and `m4_mul_dir()` functions do a correct perspective
  divide (division by w) when necessary. This is a bit slower but ensures that
  the functions will properly work with projection matrices. If profiling shows
  this is a bottleneck special functions without perspective division can be
  added. But the normal multiplications should avoid any surprises.
- The library consistently uses a right-handed coordinate system. The old
  `glOrtho()` broke that rule and `m4_ortho()` has be slightly modified so you
  can always think of right-handed cubes that are projected into OpenGLs
  normalized device coordinates.
- Special care has been taken to document all complex operations and important
  sources. Most code is covered by test cases that have been manually calculated
  and checked on the whiteboard. Since indices and math code is prone to be
  confusing we used pair programming to avoid mistakes.


FURTHER IDEARS

These are ideas for future work on the library. They're implemented as soon as
there is a proper use case and we can find good names for them.

- bool v3_is_null(vec3_t v, float epsilon)
  To check if the length of a vector is smaller than `epsilon`.
- vec3_t v3_length_default(vec3_t v, float default_length, float epsilon)
  Returns `default_length` if the length of `v` is smaller than `epsilon`.
  Otherwise same as `v3_length()`.
- vec3_t v3_norm_default(vec3_t v, vec3_t default_vector, float epsilon)
  Returns `default_vector` if the length of `v` is smaller than `epsilon`.
  Otherwise the same as `v3_norm()`.
- mat4_t m4_invert(mat4_t matrix)
  Matrix inversion that works with arbitrary matrices. `m4_invert_affine()` can
  already invert translation, rotation, scaling, mirroring, reflection and
  shearing matrices. So a general inversion might only be useful to invert
  projection matrices for picking. But with orthographic and perspective
  projection it's probably simpler to calculate the ray into the scene directly
  based on the screen coordinates.


VERSION HISTORY

v1.0  2016-02-15  Initial release

**/

#ifndef MATH_3D_HEADER
#define MATH_3D_HEADER

#include <math.h>
#include <stdio.h>


// Define PI directly because we would need to define the _BSD_SOURCE or
// _XOPEN_SOURCE feature test macros to get it from math.h. That would be a
// rather harsh dependency. So we define it directly if necessary.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


//
// 3D vectors
// 
// Use the `vec3()` function to create vectors. All other vector functions start
// with the `v3_` prefix.
// 
// The binary layout is the same as in GLSL and everything else (just 3 floats).
// So you can just upload the vectors into shaders as they are.
//

typedef struct { float x, y, z; } vec3_t;
static inline vec3_t vec3(float x, float y, float z)        { return (vec3_t){ x, y, z }; }

static inline vec3_t v3_add   (vec3_t a, vec3_t b)          { return (vec3_t){ a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline vec3_t v3_adds  (vec3_t a, float s)           { return (vec3_t){ a.x + s,   a.y + s,   a.z + s   }; }
static inline vec3_t v3_sub   (vec3_t a, vec3_t b)          { return (vec3_t){ a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline vec3_t v3_subs  (vec3_t a, float s)           { return (vec3_t){ a.x - s,   a.y - s,   a.z - s   }; }
static inline vec3_t v3_mul   (vec3_t a, vec3_t b)          { return (vec3_t){ a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline vec3_t v3_muls  (vec3_t a, float s)           { return (vec3_t){ a.x * s,   a.y * s,   a.z * s   }; }
static inline vec3_t v3_div   (vec3_t a, vec3_t b)          { return (vec3_t){ a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline vec3_t v3_divs  (vec3_t a, float s)           { return (vec3_t){ a.x / s,   a.y / s,   a.z / s   }; }
static inline float  v3_length(vec3_t v)                    { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);          }
static inline vec3_t v3_norm  (vec3_t v);
static inline float  v3_dot   (vec3_t a, vec3_t b)          { return a.x*b.x + a.y*b.y + a.z*b.z;                 }
static inline vec3_t v3_proj  (vec3_t v, vec3_t onto);
static inline vec3_t v3_cross (vec3_t a, vec3_t b);
static inline float  v3_angle_between(vec3_t a, vec3_t b);


//
// 4x4 matrices
//
// Use the `mat4()` function to create a matrix. You can write the matrix
// members in the same way as you would write them on paper or on a whiteboard:
// 
// mat4_t m = mat4(
//     1,  0,  0,  7,
//     0,  1,  0,  5,
//     0,  0,  1,  3,
//     0,  0,  0,  1
// )
// 
// This creates a matrix that translates points by vec3(7, 5, 3). All other
// matrix functions start with the `m4_` prefix. Among them functions to create
// identity, translation, rotation, scaling and projection matrices.
// 
// The matrix is stored in column-major order, just as OpenGL expects. Members
// can be accessed by indices or member names. When you write a matrix on paper
// or on the whiteboard the indices and named members correspond to these
// positions:
// 
// | m[0][0]  m[1][0]  m[2][0]  m[3][0] |
// | m[0][1]  m[1][1]  m[2][1]  m[3][1] |
// | m[0][2]  m[1][2]  m[2][2]  m[3][2] |
// | m[0][3]  m[1][3]  m[2][3]  m[3][3] |
// 
// | m00  m10  m20  m30 |
// | m01  m11  m21  m31 |
// | m02  m12  m22  m32 |
// | m03  m13  m23  m33 |
// 
// The first index or number in a name denotes the column, the second the row.
// So m[i][j] denotes the member in the ith column and the jth row. This is the
// same as in GLSL (source: GLSL v1.3 specification, 5.6 Matrix Components).
//

typedef union {
	// The first index is the column index, the second the row index. The memory
	// layout of nested arrays in C matches the memory layout expected by OpenGL.
	float m[4][4];
	// OpenGL expects the first 4 floats to be the first column of the matrix.
	// So we need to define the named members column by column for the names to
	// match the memory locations of the array elements.
	struct {
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};
} mat4_t;

static inline mat4_t mat4(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33
);

static inline mat4_t m4_identity     ();
static inline mat4_t m4_translation  (vec3_t offset);
static inline mat4_t m4_scaling      (vec3_t scale);
static inline mat4_t m4_rotation_x   (float angle_in_rad);
static inline mat4_t m4_rotation_y   (float angle_in_rad);
static inline mat4_t m4_rotation_z   (float angle_in_rad);
              mat4_t m4_rotation     (float angle_in_rad, vec3_t axis);

              mat4_t m4_ortho        (float left, float right, float bottom, float top, float back, float front);
              mat4_t m4_perspective  (float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance);
              mat4_t m4_look_at      (vec3_t from, vec3_t to, vec3_t up);

static inline mat4_t m4_transpose    (mat4_t matrix);
static inline mat4_t m4_mul          (mat4_t a, mat4_t b);
              mat4_t m4_invert_affine(mat4_t matrix);
              vec3_t m4_mul_pos      (mat4_t matrix, vec3_t position);
              vec3_t m4_mul_dir      (mat4_t matrix, vec3_t direction);

              void   m4_print        (mat4_t matrix);
              void   m4_printp       (mat4_t matrix, int width, int precision);
              void   m4_fprint       (FILE* stream, mat4_t matrix);
              void   m4_fprintp      (FILE* stream, mat4_t matrix, int width, int precision);



//
// 3D vector functions header implementation
//

static inline vec3_t v3_norm(vec3_t v) {
	float len = v3_length(v);
	if (len > 0)
		return (vec3_t){ v.x / len, v.y / len, v.z / len };
	else
		return (vec3_t){ 0, 0, 0};
}

static inline vec3_t v3_proj(vec3_t v, vec3_t onto) {
	return v3_muls(onto, v3_dot(v, onto) / v3_dot(onto, onto));
}

static inline vec3_t v3_cross(vec3_t a, vec3_t b) {
	return (vec3_t){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

static inline float v3_angle_between(vec3_t a, vec3_t b) {
	return acosf( v3_dot(a, b) / (v3_length(a) * v3_length(b)) );
}


//
// Matrix functions header implementation
//

static inline mat4_t mat4(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33
) {
	return (mat4_t){
		.m[0][0] = m00, .m[1][0] = m10, .m[2][0] = m20, .m[3][0] = m30,
		.m[0][1] = m01, .m[1][1] = m11, .m[2][1] = m21, .m[3][1] = m31,
		.m[0][2] = m02, .m[1][2] = m12, .m[2][2] = m22, .m[3][2] = m32,
		.m[0][3] = m03, .m[1][3] = m13, .m[2][3] = m23, .m[3][3] = m33
	};
}

static inline mat4_t m4_identity() {
	return mat4(
		 1,  0,  0,  0,
		 0,  1,  0,  0,
		 0,  0,  1,  0,
		 0,  0,  0,  1
	);
}

static inline mat4_t m4_translation(vec3_t offset) {
	return mat4(
		 1,  0,  0,  offset.x,
		 0,  1,  0,  offset.y,
		 0,  0,  1,  offset.z,
		 0,  0,  0,  1
	);
}

static inline mat4_t m4_scaling(vec3_t scale) {
	float x = scale.x, y = scale.y, z = scale.z;
	return mat4(
		 x,  0,  0,  0,
		 0,  y,  0,  0,
		 0,  0,  z,  0,
		 0,  0,  0,  1
	);
}

static inline mat4_t m4_rotation_x(float angle_in_rad) {
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return mat4(
		1,  0,  0,  0,
		0,  c, -s,  0,
		0,  s,  c,  0,
		0,  0,  0,  1
	);
}

static inline mat4_t m4_rotation_y(float angle_in_rad) {
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return mat4(
		 c,  0,  s,  0,
		 0,  1,  0,  0,
		-s,  0,  c,  0,
		 0,  0,  0,  1
	);
}

static inline mat4_t m4_rotation_z(float angle_in_rad) {
	float s = sinf(angle_in_rad), c = cosf(angle_in_rad);
	return mat4(
		 c, -s,  0,  0,
		 s,  c,  0,  0,
		 0,  0,  1,  0,
		 0,  0,  0,  1
	);
}

static inline mat4_t m4_transpose(mat4_t matrix) {
	return mat4(
		matrix.m00, matrix.m01, matrix.m02, matrix.m03,
		matrix.m10, matrix.m11, matrix.m12, matrix.m13,
		matrix.m20, matrix.m21, matrix.m22, matrix.m23,
		matrix.m30, matrix.m31, matrix.m32, matrix.m33
	);
}

/**
 * Multiplication of two 4x4 matrices.
 * 
 * Implemented by following the row times column rule and illustrating it on a
 * whiteboard with the proper indices in mind.
 * 
 * Further reading: https://en.wikipedia.org/wiki/Matrix_multiplication
 * But note that the article use the first index for rows and the second for
 * columns.
 */
static inline mat4_t m4_mul(mat4_t a, mat4_t b) {
	mat4_t result;
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			float sum = 0;
			for(int k = 0; k < 4; k++) {
				sum += a.m[k][j] * b.m[i][k];
			}
			result.m[i][j] = sum;
		}
	}
	
	return result;
}

#endif // MATH_3D_HEADER


#ifdef MATH_3D_IMPLEMENTATION

/**
 * Creates a matrix to rotate around an axis by a given angle. The axis doesn't
 * need to be normalized.
 * 
 * Sources:
 * 
 * https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
 */
mat4_t m4_rotation(float angle_in_rad, vec3_t axis) {
	vec3_t normalized_axis = v3_norm(axis);
	float x = normalized_axis.x, y = normalized_axis.y, z = normalized_axis.z;
	float c = cosf(angle_in_rad), s = sinf(angle_in_rad);
	
	return mat4(
		c + x*x*(1-c),            x*y*(1-c) - z*s,      x*z*(1-c) + y*s,  0,
		    y*x*(1-c) + z*s,  c + y*y*(1-c),            y*z*(1-c) - x*s,  0,
		    z*x*(1-c) - y*s,      z*y*(1-c) + x*s,  c + z*z*(1-c),        0,
		    0,                        0,                    0,            1
	);
}


/**
 * Creates an orthographic projection matrix. It maps the right handed cube
 * defined by left, right, bottom, top, back and front onto the screen and
 * z-buffer. You can think of it as a cube you move through world or camera
 * space and everything inside is visible.
 * 
 * This is slightly different from the traditional glOrtho() and from the linked
 * sources. These functions require the user to negate the last two arguments
 * (creating a left-handed coordinate system). We avoid that here so you can
 * think of this function as moving a right-handed cube through world space.
 * 
 * The arguments are ordered in a way that for each axis you specify the minimum
 * followed by the maximum. Thats why it's bottom to top and back to front.
 * 
 * Implementation details:
 * 
 * To be more exact the right-handed cube is mapped into normalized device
 * coordinates, a left-handed cube where (-1 -1) is the lower left corner,
 * (1, 1) the upper right corner and a z-value of -1 is the nearest point and
 * 1 the furthest point. OpenGL takes it from there and puts it on the screen
 * and into the z-buffer.
 * 
 * Sources:
 * 
 * https://msdn.microsoft.com/en-us/library/windows/desktop/dd373965(v=vs.85).aspx
 * https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
 */
mat4_t m4_ortho(float left, float right, float bottom, float top, float back, float front) {
	float l = left, r = right, b = bottom, t = top, n = front, f = back;
	float tx = -(r + l) / (r - l);
	float ty = -(t + b) / (t - b);
	float tz = -(f + n) / (f - n);
	return mat4(
		 2 / (r - l),  0,            0,            tx,
		 0,            2 / (t - b),  0,            ty,
		 0,            0,            2 / (f - n),  tz,
		 0,            0,            0,            1
	);
}

/**
 * Creates a perspective projection matrix for a camera.
 * 
 * The camera is at the origin and looks in the direction of the negative Z axis.
 * `near_view_distance` and `far_view_distance` have to be positive and > 0.
 * They are distances from the camera eye, not values on an axis.
 * 
 * `near_view_distance` can be small but not 0. 0 breaks the projection and
 * everything ends up at the max value (far end) of the z-buffer. Making the
 * z-buffer useless.
 * 
 * The matrix is the same as `gluPerspective()` builds. The view distance is
 * mapped to the z-buffer with a reciprocal function (1/x). Therefore the z-buffer
 * resolution for near objects is very good while resolution for far objects is
 * limited.
 * 
 * Sources:
 * 
 * https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
 */
mat4_t m4_perspective(float vertical_field_of_view_in_deg, float aspect_ratio, float near_view_distance, float far_view_distance) {
	float fovy_in_rad = vertical_field_of_view_in_deg / 180 * M_PI;
	float f = 1.0f / tanf(fovy_in_rad / 2.0f);
	float ar = aspect_ratio;
	float nd = near_view_distance, fd = far_view_distance;
	
	return mat4(
		 f / ar,           0,                0,                0,
		 0,                f,                0,                0,
		 0,                0,               (fd+nd)/(nd-fd),  (2*fd*nd)/(nd-fd),
		 0,                0,               -1,                0
	);
}

/**
 * Builds a transformation matrix for a camera that looks from `from` towards
 * `to`. `up` defines the direction that's upwards for the camera. All three
 * vectors are given in world space and `up` doesn't need to be normalized.
 * 
 * Sources: Derived on whiteboard.
 * 
 * Implementation details:
 * 
 * x, y and z are the right-handed base vectors of the cameras subspace.
 * x has to be normalized because the cross product only produces a normalized
 *   output vector if both input vectors are orthogonal to each other. And up
 *   probably isn't orthogonal to z.
 * 
 * These vectors are then used to build a 3x3 rotation matrix. This matrix
 * rotates a vector by the same amount the camera is rotated. But instead we
 * need to rotate all incoming vertices backwards by that amount. That's what a
 * camera matrix is for: To move the world so that the camera is in the origin.
 * So we take the inverse of that rotation matrix and in case of an rotation
 * matrix this is just the transposed matrix. That's why the 3x3 part of the
 * matrix are the x, y and z vectors but written horizontally instead of
 * vertically.
 * 
 * The translation is derived by creating a translation matrix to move the world
 * into the origin (thats translate by minus `from`). The complete lookat matrix
 * is then this translation followed by the rotation. Written as matrix
 * multiplication:
 * 
 *   lookat = rotation * translation
 * 
 * Since we're right-handed this equals to first doing the translation and after
 * that doing the rotation. During that multiplication the rotation 3x3 part
 * doesn't change but the translation vector is multiplied with each rotation
 * axis. The dot product is just a more compact way to write the actual
 * multiplications.
 */
mat4_t m4_look_at(vec3_t from, vec3_t to, vec3_t up) {
	vec3_t z = v3_muls(v3_norm(v3_sub(to, from)), -1);
	vec3_t x = v3_norm(v3_cross(up, z));
	vec3_t y = v3_cross(z, x);
	
	return mat4(
		x.x, x.y, x.z, -v3_dot(from, x),
		y.x, y.y, y.z, -v3_dot(from, y),
		z.x, z.y, z.z, -v3_dot(from, z),
		0,   0,   0,    1
	);
}


/**
 * Inverts an affine transformation matrix. That are translation, scaling,
 * mirroring, reflection, rotation and shearing matrices or any combination of
 * them.
 * 
 * Implementation details:
 * 
 * - Invert the 3x3 part of the 4x4 matrix to handle rotation, scaling, etc.
 *   correctly (see source).
 * - Invert the translation part of the 4x4 matrix by multiplying it with the
 *   inverted rotation matrix and negating it.
 * 
 * When a 3D point is multiplied with a transformation matrix it is first
 * rotated and then translated. The inverted transformation matrix is the
 * inverse translation followed by the inverse rotation. Written as a matrix
 * multiplication (remember, the effect applies right to left):
 * 
 *   inv(matrix) = inv(rotation) * inv(translation)
 * 
 * The inverse translation is a translation into the opposite direction, just
 * the negative translation. The rotation part isn't changed by that
 * multiplication but the translation part is multiplied by the inverse rotation
 * matrix. It's the same situation as with `m4_look_at()`. But since we don't
 * store the rotation matrix as 3D vectors we can't use the dot product and have
 * to write the matrix multiplication operations by hand.
 * 
 * Sources for 3x3 matrix inversion:
 * 
 * https://www.khanacademy.org/math/precalculus/precalc-matrices/determinants-and-inverses-of-large-matrices/v/inverting-3x3-part-2-determinant-and-adjugate-of-a-matrix
 */
mat4_t m4_invert_affine(mat4_t matrix) {
	// Create shorthands to access matrix members
	float m00 = matrix.m00,  m10 = matrix.m10,  m20 = matrix.m20,  m30 = matrix.m30;
	float m01 = matrix.m01,  m11 = matrix.m11,  m21 = matrix.m21,  m31 = matrix.m31;
	float m02 = matrix.m02,  m12 = matrix.m12,  m22 = matrix.m22,  m32 = matrix.m32;
	
	// Invert 3x3 part of the 4x4 matrix that contains the rotation, etc.
	// That part is called R from here on.
		
		// Calculate cofactor matrix of R
		float c00 =   m11*m22 - m12*m21,   c10 = -(m01*m22 - m02*m21),  c20 =   m01*m12 - m02*m11;
		float c01 = -(m10*m22 - m12*m20),  c11 =   m00*m22 - m02*m20,   c21 = -(m00*m12 - m02*m10);
		float c02 =   m10*m21 - m11*m20,   c12 = -(m00*m21 - m01*m20),  c22 =   m00*m11 - m01*m10;
		
		// Caclculate the determinant by using the already calculated determinants
		// in the cofactor matrix.
		// Second sign is already minus from the cofactor matrix.
		float det = m00*c00 + m10*c10 + m20 * c20;
		if (fabsf(det) < 0.00001)
			return m4_identity();
		
		// Calcuate inverse of R by dividing the transposed cofactor matrix by the
		// determinant.
		float i00 = c00 / det,  i10 = c01 / det,  i20 = c02 / det;
		float i01 = c10 / det,  i11 = c11 / det,  i21 = c12 / det;
		float i02 = c20 / det,  i12 = c21 / det,  i22 = c22 / det;
	
	// Combine the inverted R with the inverted translation
	return mat4(
		i00, i10, i20,  -(i00*m30 + i10*m31 + i20*m32),
		i01, i11, i21,  -(i01*m30 + i11*m31 + i21*m32),
		i02, i12, i22,  -(i02*m30 + i12*m31 + i22*m32),
		0,   0,   0,      1
	);
}

/**
 * Multiplies a 4x4 matrix with a 3D vector representing a point in 3D space.
 * 
 * Before the matrix multiplication the vector is first expanded to a 4D vector
 * (x, y, z, 1). After the multiplication the vector is reduced to 3D again by
 * dividing through the 4th component (if it's not 0 or 1).
 */
vec3_t m4_mul_pos(mat4_t matrix, vec3_t position) {
	vec3_t result = vec3(
		matrix.m00 * position.x + matrix.m10 * position.y + matrix.m20 * position.z + matrix.m30,
		matrix.m01 * position.x + matrix.m11 * position.y + matrix.m21 * position.z + matrix.m31,
		matrix.m02 * position.x + matrix.m12 * position.y + matrix.m22 * position.z + matrix.m32
	);
	
	float w = matrix.m03 * position.x + matrix.m13 * position.y + matrix.m23 * position.z + matrix.m33;
	if (w != 0 && w != 1)
		return vec3(result.x / w, result.y / w, result.z / w);
	
	return result;
}

/**
 * Multiplies a 4x4 matrix with a 3D vector representing a direction in 3D space.
 * 
 * Before the matrix multiplication the vector is first expanded to a 4D vector
 * (x, y, z, 0). For directions the 4th component is set to 0 because directions
 * are only rotated, not translated. After the multiplication the vector is
 * reduced to 3D again by dividing through the 4th component (if it's not 0 or
 * 1). This is necessary because the matrix might contains something other than
 * (0, 0, 0, 1) in the bottom row which might set w to something other than 0
 * or 1.
 */
vec3_t m4_mul_dir(mat4_t matrix, vec3_t direction) {
	vec3_t result = vec3(
		matrix.m00 * direction.x + matrix.m10 * direction.y + matrix.m20 * direction.z,
		matrix.m01 * direction.x + matrix.m11 * direction.y + matrix.m21 * direction.z,
		matrix.m02 * direction.x + matrix.m12 * direction.y + matrix.m22 * direction.z
	);
	
	float w = matrix.m03 * direction.x + matrix.m13 * direction.y + matrix.m23 * direction.z;
	if (w != 0 && w != 1)
		return vec3(result.x / w, result.y / w, result.z / w);
	
	return result;
}

void m4_print(mat4_t matrix) {
	m4_fprintp(stdout, matrix, 6, 2);
}

void m4_printp(mat4_t matrix, int width, int precision) {
	m4_fprintp(stdout, matrix, width, precision);
}

void m4_fprint(FILE* stream, mat4_t matrix) {
	m4_fprintp(stream, matrix, 6, 2);
}

void m4_fprintp(FILE* stream, mat4_t matrix, int width, int precision) {
	mat4_t m = matrix;
	int w = width, p = precision;
	for(int r = 0; r < 4; r++) {
		fprintf(stream, "| %*.*f %*.*f %*.*f %*.*f |\n",
			w, p, m.m[0][r], w, p, m.m[1][r], w, p, m.m[2][r], w, p, m.m[3][r]
		);
	}
}

#endif // MATH_3D_IMPLEMENTATION