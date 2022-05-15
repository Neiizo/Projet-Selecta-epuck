#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

float get_distance_cm(void);
void process_image_start(void);
void detect_codebarre(uint32_t width);
uint16_t get_line_pos(void);
uint8_t get_code_bar(void);

#endif /* PROCESS_IMAGE_H */
