# 3d-demo
OpenGL playground

Auto exposure

GLfloat luminescence[3];
glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, &luminescence);
GLfloat lum = 0.2126 * luminescence[0] + 0.7152 * luminescence[1] + 0.0722 * luminescence[2];
exposure = lerp(exposure, 0.5 / lum, 0.05); // slowly adjust exposure based on average brightness
