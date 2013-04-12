#include "stdafx.h"
#include "LightMap.h"

static float light_pos[3] = { 1.0f, 0.0f, 0.25f };
static float light_color[3] = { 1.0f, 1.0f, 1.0f };
#define LIGHTMAP_SIZE 1024;

//unsigned int generate_lightmap(struct surface *surf)
//{
//	static unsigned char data[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 3];
//	static unsigned int lightmap_tex_num = 0;
//	unsigned int i, j;
//	float pos[3];
//	float step, s, t;
//
//	//if(lightmap_tex_num == 0)
//		//glGenTextures(1, &lightmap_tex_num);
//
//	step = 1.0f / LIGHTMAP_SIZE;
//
//	s = t = 0.0f;
//	for(i = 0; i < LIGHTMAP_SIZE; i++) {
//		for(j = 0; j < LIGHTMAP_SIZE; j++) {
//			float d;
//			float tmp;
//
//			pos[0] = surf->s_dist * s;
//			pos[1] = surf->t_dist * t;
//			pos[2] = 0.0f;
//
//			//multiply_vector_by_matrix(surf->matrix, pos);
//			pos[0] += surf->vertices[0][0];
//			pos[1] += surf->vertices[0][1];
//			pos[2] += surf->vertices[0][2];
//			pos[0] -= light_pos[0];
//			pos[1] -= light_pos[1];
//			pos[2] -= light_pos[2];
//
//			/*d = dot_product(pos, pos) * 0.5f;
//			if(d < 1.0f)
//				d = 1.0f;
//			tmp = 1.0f / d;
//
//			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 0] = (unsigned char)(255.0f * tmp * light_color[0]);
//			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 1] = (unsigned char)(255.0f * tmp * light_color[1]);
//			data[i * LIGHTMAP_SIZE * 3 + j * 3 + 2] = (unsigned char)(255.0f * tmp * light_color[2]);
//*/
//
//			s += step;
//		}
//
//		t += step;
//		s = 0.0f;
//
//	}
//
//	/*glBindTexture(GL_TEXTURE_2D, lightmap_tex_num);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	glTexImage2D(GL_TEXTURE_2D, 0, 3, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	*/
//
//	return lightmap_tex_num;
//}
