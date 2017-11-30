# Tasks

## Matt
- [ ] Compass
- [ ] Checkoff


## Sydney
- [ ] Ranger
- [ ] PCA_ISR from Lab 6
- [ ] Checkoff

## Tom
- [ ] Setup
- [ ] Checkoff


# Lab_6
LITEC Lab 6

## Project Requirements

### Hardware
1. 


### Software
1. Ranger controls the desired heading of the gondola.
2. Compass controls the spin direction of the tail fans.
3. Gains entered with the keypad or keyboard (*as in one or the other, or as in both must be functioning?*)
4. Desired heading initially set using keypad or keyboard, but modified using the ranger. There should be a **neutral height** setting to work around.
5. Pulse width for tail fan based on error of desired heading and actual heading (which is measured by the compass).

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

```C

```

```C

```

### Gondola Behavior
1. 


## Other Considerations
1. 

```C

```

# Completed Tasks

## Matt
- [x] Refactor PCA_ISR logic for reading flags


## Sydney
- [x] Refactor Print_Data


## Tom
- [x] Determine main loop logic
