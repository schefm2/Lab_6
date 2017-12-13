# Tasks
## Lab 5
### Notebook
- [ ] Lab Title
- [ ] Lab Description
- [ ] Worksheets
- [x] Pseudo-code - Matt
- [x] Circuit Schematics - Matt
- [ ] Data & Notes
- [ ] Data Analysis
- [x] Computer Code

### Design Report
- [ ] Introduction
- [ ] Description of Goal Achieved
- [ ] Verification
- [ ] Data Plots
- [ ] Data Analysis
- [ ] What Was Learned
- [ ] Problems Encountered & Solutions
- [ ] Suggested Improvements to HW & SW (Our end, not theirs)
- [x] Code Upload - Matt
- [ ] Formatting & Neatness
- [ ] Division of Labor Page
- [ ] HW #12 - Matt, 

## Lab 6
### Notebook
- [ ] Lab Title
- [ ] Lab Description
- [ ] Worksheets
- [x] Pseudo-code - Matt
- [x] Circuit Schematics - Matt
- [ ] Data & Notes
- [ ] Data Analysis
- [x] Computer Code - Matt

### Design Report
- [ ] Introduction
- [ ] Circuit Schematic
- [ ] Description of Gondola performance
- [ ] Verification
- [ ] Data Plots
- [ ] Data Analysis
- [ ] What Was Learned
- [ ] Problems Encountered & Solutions
- [x] Pseudocode - Matt
- [x] Code Upload - Matt
- [ ] Formatting & Neatness
- [ ] Division of Labor Page
- [ ] HW #12 - Matt, 

## Matt
- [ ] Compiling lab notebook
- [ ] Working on design reports

## Sydney
- [ ] Comment code for Lab 6
- [ ] Lab notebook and design report descriptions


## Tom
- [ ] Comment code for Lab 5 (**there are errors!**)
- [ ] Update pseudocode for Lab 5 (slightly out of date atm)
- [ ] Data Analysis for design reports and lab notebook


# Lab_6
LITEC Lab 6

## Project Requirements
### Hardware
The hardware of this project is already constructed.


### Software
1. Ranger controls the desired heading of the gondola.
2. Compass controls the spin direction of the tail fans.
3. Gains entered with the keypad or keyboard (*as in one or the other, or as in both must be functioning?*)
4. Desired heading initially set using keypad or keyboard, but modified using the ranger. There should be a **neutral height** setting to work around.
5. Pulse width for tail fan based on error of desired heading and actual heading (which is measured by the compass).
6. Servo controls the thrust fan angles
7. Thrust fan angles will be set so that the fans are vertical to the table, and used to turn the gondola instead of the tail fan. **The two thrust fans must spin in opposite directions!**
8. For this semester, **the tail fan will not be used!**
9. **Neutral height will be set to 50 [cm] with a small deadband**. Increasing the ranger distance will turn the gondola clockwise to a max offset of 180 degrees, and decreasing the ranger distance will turn it counterclockwise to -180 degrees max.
10. **Include a routine for adjusting the thrust angle to vertical**, as it will be different for each gondola.
11. thrust fans will spin in opposite directions, but the correct direction for the fans must be determined based on the heading error. 
12. Using a XBAR of 0x25 (P0.4 to P0.7), CCM0 controls the rudder fan (**not used in this lab**), CCM1 controls the thrust angle, CCM2 controls left fan power, and CCM3 controls right fan power.
13. A/D conversion of the battery voltage will take place on P1.3. The voltage divider is set up such that the ADC will read a voltage that is .236*(Battery Voltage).


More specifics:
COMPASS
1. Heading control: PD algorithm changes thrust fans
2. Include routine to allow for manual adjustment of fan angle such that fans are vertical during initializations
3. Test using different P D gains using Wkst11 control equation
4. P only should apply correct signal to fans for full 360 rotation. D only works for +-angular velocity for full 360.
5. Record actual heading vs time; plot

RANGER
1. Set PWM signal for 2 thrust fans (spin in opposite directions, correct direciton dependent on thrust angle)
2. Adjust/control desired heading dependent on ranger above/below nominal value (~50cm)
3. User adjust PW for faster/slower heading error correction based on feedback gains
4. Test with hand above ranger
5. Print heading values (vs time?)

OTHER/Joint?
1. Use LCD/keypad or RF to set: init desired heading, heading proportional gain, heading derivative gain
2. nominal altitude doesn't adjust init heading can be fixed, should be ~50cm
3. Print desired heading, actual heading, ranger heading, heading angle adjustment, thrust PW, battery voltage.
    Occasionally print modified desired heading to save time/reduce clutter; print as CSV


### Data Acquisition
1. Turn off all fans and print data for a full 360 degree spin. Set the derivative gain to 0. When heading error reaches 180 degrees, it should flip its sign. This is good for detecting errors in heading calculations.
2. A second data set during regular operation, use the following different gain combinations:
>Without differential gain:
> 1. kp = 0.1, kd = 0
> 2. kp = 5, kd = 0
>
>With differential gain:
> 1. kp = 0.1, kd = 10
> 2. kp = 0.5, kd = 70
> 3. kp = 3, kd = 70
> 4. kp = 3, kd = 180
> 5. kp = 12, kd = 70
> 6. kp = 12, kd = 180
>
>Additionally, an optimal gain combination should be found.
>
>*Use other gain values if you cannot get all of the types of responses—critically damped, underdamped, overdamped, and unstable.*




## Other Considerations
1. The fans have greater thrust in the forward direction than in the reverse.
2. Type casting for signed longs on the right side of Pulse Width setting equations is recommended, as this lab works with very large gains.

An example using Kp as the proportional gain and Kd as the differential gain (with rudder_pw declared as unsigned int). tmp_pw must be a signed long to handle large or negative values.
```C
error = desired_heading–heading;
tmp_pw = (long)Kp*error+(long)Kd*(error-previous_error)+RUDDER_CENTER;
if (tmp_pw > (long)RUDDER_LEFT) tmp_pw = RUDDER_LEFT;
else if (tmp_pw < (long)RUDDER_RIGHT) tmp_pw = RUDDER_RIGHT;
rudder_pw = (unsigned int)tmp_pw;
previous_error = error;
```
3. One may consider smoothing out the heading readings by averaging a few values (*save this for later?*)
4. Start with only P control, then add in the D control.
5. Display battery voltage in mV on LCD and terminal to avoid the use of floats.
6. Optional use of only the tail fan to get the speed of the gondola's rotation down to a certain value, then switching to standard PD control.

# Completed Tasks
## Matt
- [x] Update `Set_Motor_PWM`
- [x] Update `Start_Parameters()`
- [x] Update `calibrate()` to allow for larger gain setting
- [x] Update `Set_Servo_PWM()`
- [x] PCA_ISR (change timing logic to Lab 5's style)
- [x] Function to change thrust angle, to be called in `Start_Parameters()`
- [x] Variable nomenclature
- [x] Determine maximum battery voltage and change `Calculate_Voltage()` accordingly
- [x] Determine why `Print_Data()` prints Motor_PW incorrectly
- [x] Worksheet 11
- [x] Pseudocode
- [x] Test ranger functionality, design solutions to its problems
- [x] Gather data for Lab 6 (gain settings, *ask if we need plots for all gain settings for both ranger and nonranger runs*)
- [x] Lab 6 Checkoff
- [x] Updated pseudocode for Lab 6
- [x] Circuit schematics for Lab 5 & 6
- [x] Ask if Lab 6 data just needs to be under, over, crit damped, unstable, optimal, and optimal with ranger

## Sydney
- [x] Reconfigure ports
- [x] Worksheet 11
- [x] Determine maximum battery voltage and change `Calculate_Voltage()` accordingly
- [x] Determine why `Print_Data()` prints Motor_PW incorrectly
- [x] Incorporate ranger to alter `desired_heading`
- [x] Test ranger functionality, design solutions to its problems
- [x] Lab 6 Checkoff

## Tom
- [x] Lorem ipsum
- [x] Worksheet 11
- [x] Determine maximum battery voltage and change `Calculate_Voltage()` accordingly
- [x] Determine why `Print_Data()` prints Motor_PW incorrectly
- [x] Lab 6 Checkoff
- [x] Test ranger functionality, design solutions to its problems