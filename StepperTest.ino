/* 
 *      Simple program for testing the stepper motor and stepper driver
 *      
 *      Start with the STEP_PULSE_WIDTH set to 20 and the INTER_PULSE_DELAY AT 500
 *      Set REVOLUTIONS to 1 then play with the STEPS_PER_REVOLUTION value
 *      until the stepper motor can be made to do one complete revolution per burst
 *      Once you have it rotating correctly gradually reduce the STEP_PULSE_WIDTH until it
 *      starts to fail to revolve a complete turn then bump it back up to a reliable value
 *      Then gradually bring the INTER_PULSE_DELAY down in the same way until it fails and then increase back
 *      to a stable value.
 *      This will give the settings for the maximum rotation speed for the motor/drive settings.
 *      
 */
 
// Arduino pin usage

// Output pins on the Arduino UNO
#define STEP_PIN        3   // Output to the PULSE input on the stepper driver unit
#define DIR_PIN         4   // Output to the DIR input on the stepper driver unit
#define ENABLE_PIN      5   // Output to the ENABLE input on the stepper driver unit

// **** IMPORTANT ****
// These are the values for the switch settings on my stepper driver
// Change them to match the values for your stepper driver
//
// #define STEPS_PER_REVOLUTION 384 // OFF ON OFF
// #define STEPS_PER_REVOLUTION 768 // OFF OFF ON
#define STEPS_PER_REVOLUTION 1536   // OFF OFF OFF

// Start-up values to give a reasonable functionality
#define REVOLUTIONS 1   // How many complete revolutions of the motor
#define NUMBER_OF_STEPS ( STEPS_PER_REVOLUTION * REVOLUTIONS )  // How many step pulses to output per loop
#define STEP_PULSE_WIDTH 20          // Width of the step pulse in microseconds
#define INTER_PULSE_DELAY 500       // Microseconds between pulses
#define INTER_PULSE_MINIMUM 10      // Minimum time between pulses
#define INTER_LOOP_DELAY 1000       // How many milliseconds between rotation bursts           
#define OVERHEAD_DELAY 4            // Step delay function overhead in microseconds

enum rotationDirection { CW, CCW } CurrentDir = CW; // Holds the current direction of rotation
volatile unsigned long int StepPulseWidth = STEP_PULSE_WIDTH - OVERHEAD_DELAY; // Set sensible initial values
volatile unsigned long int InterPulseDelay = INTER_PULSE_DELAY;
volatile unsigned long int NumberOfSteps = NUMBER_OF_STEPS;

void setup() 
{

    Serial.begin( 115200 ); // Initialise the serial port
    
    // Set the Arduino pins as inputs or outputs as required
    pinMode( STEP_PIN, OUTPUT );
    pinMode( DIR_PIN, OUTPUT );
    pinMode( ENABLE_PIN, OUTPUT );
    setEnable( false );
    // Now print the instructions for usage
    Serial.println( "Usage:" );
    Serial.println( "W xxx to change pulse width" );
    Serial.println( "P xxx to change inter-pulse period" );
    Serial.println( "  - where xxx is a time in microseconds" );
    Serial.println( "  Example W 100 is 100uSec pulse width and P 2000000 is 2 seconds between pulses" );
    Serial.println( "N xxx to change number of steps" );
    Serial.println( "D to change rotation direction" );
    Serial.println( "? to print current values" );
    Serial.println( "ok>" );
}

void loop() 
{   
    doStep();
    delay( INTER_LOOP_DELAY );
}

// Enable/disable the stepper motor driver
void setEnable( bool State )
{
    if ( State == true )
    {
        digitalWrite( ENABLE_PIN, LOW );
     }
    else
    {
        digitalWrite( ENABLE_PIN, HIGH );
    }
}

// Set the stepper motor rotation direction on the stepper motor driver
void setDirection( rotationDirection Direction )
{
    if ( Direction == CW )
    {
        digitalWrite( DIR_PIN, HIGH );           
    }
    else
    {
        digitalWrite( DIR_PIN, LOW );
    }

}

// Run the stepper for NumberOfSteps with StepPulseWidth and InterPulseDelay
void doStep()
{
    setEnable( true );
    setDirection( CurrentDir );
    for ( unsigned long int x = 0; x < NumberOfSteps; ++x )
    {
        digitalWrite( STEP_PIN, HIGH );
        delayMicroseconds( StepPulseWidth ); // There is a 4uS additional time taken in writing the pins
        digitalWrite( STEP_PIN, LOW ); 
        delayMicroseconds( InterPulseDelay );
    }
    setEnable( false );   
}

// This method gets called when serial data is available
void serialEvent()
{
    char buffer[ 40 ] = { 0 };  // String to hold the formatted data. Keep an eye on this to make sure we don't exceed it

    int iCommand = Serial.peek(); // Examine the first character to see if it is a command
    unsigned long l = Serial.parseInt(); // Get any numeric values if present

    if ( Serial.available() > 0 )
        Serial.readString(); // Flush the input buffer of anything left by Serial.parseInt() function       

    // Here is where we parse the command and carry out the requested task
    switch ( iCommand ) // If command valid then process it
    {
        case 0x77: // W & w
        case 0x57:  if ( l > OVERHEAD_DELAY ) // Check the size of pulse requested
                    {
                        StepPulseWidth = ( l - OVERHEAD_DELAY ); // Remove the overhead to get the correct width
                        sprintf( buffer, "Pulse width set to %u uSec", l);
                    }
                   else
                       sprintf( buffer, "ERROR: 5 uSec is the minimum" );                   
            break;
            
        case 0x70: // P & p
        case 0x50: InterPulseDelay = ( l > INTER_PULSE_MINIMUM ) ? l : INTER_PULSE_MINIMUM; // Don't go below INTER_PULSE_MINIMUM
                    sprintf( buffer, "Inter pulse delay set to %u uSec", InterPulseDelay );
            break;
 
        case 0x6E: // N & n 
        case 0x4E:  NumberOfSteps = ( l > 0 ) ? l : 1;  // Always at least one step
                    sprintf( buffer, "Number of steps set to %u", NumberOfSteps );
            break;
              
        case 0x3F:  // ?
                    printValues(); // This is the ? command
                    buffer[ 0 ] = 0; // Clear the buffer to avoid a bug in the compiler
            break; 

        case 0x44:  // D & d
        case 0x64:  CurrentDir = ( CurrentDir == CW ) ? CCW:CW; // Change direction of rotation 
                    sprintf( buffer, "Now rotating %s", (CurrentDir == CW)?"Clockwise":"Counter Clockwise" );      
            break; 

        default:   sprintf( buffer, "ERROR: Command not recognised - %c", iCommand );
            break;        
    }
    if ( buffer[ 0 ] != 0 )
        Serial.println( buffer );
    Serial.println( "ok>" );    
}

void printValues()
{
    char buffer[ 40 ]; // String to hold the formatted text.
    Serial.println( "--------------------------------------------" );
    sprintf( buffer, "Pulse width %u uSec", (StepPulseWidth + OVERHEAD_DELAY ));
    Serial.println( buffer );
    sprintf( buffer, "Inter pulse delay %u uSec",InterPulseDelay );
    Serial.println( buffer );
    sprintf( buffer, "Number of steps %u", NumberOfSteps );
    Serial.println( buffer );
    sprintf( buffer, "Now rotating %s", (CurrentDir == CW)?"Clockwise":"Counter Clockwise" );
    Serial.println( buffer );
    Serial.println( "--------------------------------------------" );
    Serial.println( "ok>" );
}

// I use this for testing sometimes. You can remove it if you like
void _serialEvent() 
{
    int i = Serial.peek();
    long l = Serial.parseInt();
    if ( Serial.available() > 0 )
        String s = Serial.readString(); // Flush the input buffer
    Serial.println( i, HEX);
}

