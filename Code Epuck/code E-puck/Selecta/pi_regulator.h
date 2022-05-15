#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

//start the PI regulator thread
void pi_regulator_start(void);
void re_enable_pi_regulator(void);

void disable_pi_regulator(void);

bool get_pi_status(void);
bool get_found_status(void);

#endif /* PI_REGULATOR_H */
