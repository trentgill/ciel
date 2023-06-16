/*
#define GPIO_OSPEEDER_OSPEEDR0_Pos       (0U)                                  
#define GPIO_OSPEEDER_OSPEEDR0_Msk       (0x3U << GPIO_OSPEEDER_OSPEEDR0_Pos)  //!< 0x00000003
#define GPIO_OSPEEDER_OSPEEDR0           GPIO_OSPEEDER_OSPEEDR0_Msk            

#define PERIPH_BASE            0x40000000U //!< Base address of : AHB/ABP Peripherals
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000U)
#define GPIOB_BASE            (AHB1PERIPH_BASE + 0x0400U)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)


#define WRITE_REG(REG, VAL) \
  ((REG) = (VAL))
#define READ_REG(REG) \
  ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) \
  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))


#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))  // count leading zeroes
*/

// this is the code-block that will be inserted when setting a single pin to be output
{ // Note: this just sets the speed 4 times, but imagine the last 3 values are different
  *0x40020408U = (*0x40020408U & 0xFFF3FFFFU) | 0x00080000U;
  *0x40020408U = (*0x40020408U & 0xFFF3FFFFU) | 0x00080000U;
  *0x40020408U = (*0x40020408U & 0xFFF3FFFFU) | 0x00080000U;
  *0x40020408U = (*0x40020408U & 0xFFF3FFFFU) | 0x00080000U;
}
// the point is there is no function call, and *no runtime computation* whatsoever.
// this is old-school bare-metal style which is blazingly fast, but famously difficult to understand
// without encyclopedic knowledge of the datasheet / reference manual (very hard with cortex M4/7 chips)

// our goal is to acheive this level of register manipulation at runtime,
// while providing a more readable, declarative syntax to the programmer
// we do that with a big layer of compile-time lua optimization & safety checks
// this should check types & parameter ranges, and have extensive testing throughout

// a core feature should be to provide generated source at multiple levels of optimization
// - high-level where the symbolic names are still in the pseudo-C AST & function calls remain
// - symbolic names remain, but fn-calls are inlined
// - symbolic names are collapsed to raw values, top-level fns remain separate
// - final form where everything is maximally collapsed.

// this can also be built gradually to minimize the overhead
// - at first we just allow lua to write the C code directly with simple syntax conversion
// - next we implement the user-facing dout library with declarative syntax (parsed at runtime on chip)
// - next we add compile-time lookup to convert the declarative strings to HAL symbols
// - then we work from the bottom-up, converting HAL functions to bitwise operations (no control flow)
// this last stage is done by converting the decision-tree into a compile time operation that generates
// the bitwise operations.
// - finally we optimize the data-representation to simplify the flow from declarative syntax through to bitbangs.

// best to start with a simple led-blink example

void HAL_GPIO_Init(GPIO_InitTypeDef *GPIO_Init)
{
  uint32_t temp = 0;

  // restriction: only allow a single pin to be set at a time
  uint32_t position = 1 << GPIO_Init->Pin;


  // This pattern of reading the current register, then writing it back -- why are we doing that?
  // If we *know* what is in those registers, we should be able to compute the new mask directly

  /* Output mode */
  /* Configure the IO Speed */
  temp = GPIOB->OSPEEDR;
  temp &= ~(GPIO_OSPEEDER_OSPEEDR0 << (position * 2)); // clearing some bit
  temp |= (GPIO_Init->Speed << (position * 2)); // writing new val to that bit
  GPIOB->OSPEEDR = temp; // re-writing the register
  // ^^^ really seems like we're just change 1 bit which doesn't depend on the previous state
  // this should not be a read-then-write, but just a bitmasked write directly to the register
  // the benefit is *not* speed, but ability to calculate the mask at compile time & save it
  // as a constant in flash.

  // boils down to this runtime action
  *0x40020408U = (*0x40020408U & 0xFFF3FFFFU) | 0x00080000U;
  // the 2 deref'd values are the address of the register to be modified
  // the & bitmask copies current state, clearing the 2 speed bits
  // the | bitwise set copies in the new value
  // Can probably do 10-20 of these calls before equaling code-size of runtime version
  // Plus this is the absolute fastest the operation can be done in C


/*
  MODIFY_REG( GPIOB->OSPEEDR
            , (GPIO_OSPEEDER_OSPEEDR0 << (POSITION_VAL(Pin) * 2U))
            , (Speed << (POSITION_VAL(Pin) * 2U)));

GPIOB = 0x40020400U
GPIOB->OSPEEDR = 0x40020408U
GPIO_OSPEEDER_OSPEEDR0 = 0x00000003U
POSITION_VAL(Pin) = 9 // actually we just supply the value 9 directly! no lookup
Speed = LL_GPIO_SPEED_FREQ_HIGH
LL_GPIO_SPEED_FREQ_HIGH = GPIO_OSPEEDER_OSPEEDR0_1
GPIO_OSPEEDER_OSPEEDR0_1 = (2 << GPIO_OSPEEDER_OSPEEDR0_Pos)  //!< 0x00000002

  MODIFY_REG( 0x40020408U
            , (0x00000003U << (9 * 2))
            , (0x00000002U << (9 * 2)));

  MODIFY_REG( 0x40020408U
            , (0x00000003U << 18)
            , (0x00000002U << 18));

  MODIFY_REG( 0x40020408U
            , (0x000C0000U)
            , (0x00080000U));

  WRITE_REG((0x40020408U), (((READ_REG(0x40020408U)) & (~(0x000C0000U))) | (0x00080000U)))

  WRITE_REG(0x40020408U, (READ_REG(0x40020408U) & 0xFFF3FFFFU) | 0x00080000U)
  */


  /* Configure the IO Output Type */
  temp = GPIOB->OTYPER;
  temp &= ~(GPIO_OTYPER_OT_0 << position) ;
  temp |= (((GPIO_Init->Mode & GPIO_OUTPUT_TYPE) >> 4) << position);
  GPIOB->OTYPER = temp;

  /* Activate the Pull-up or Pull down resistor for the current IO */
  temp = GPIOB->PUPDR;
  temp &= ~(GPIO_PUPDR_PUPDR0 << (position * 2));
  temp |= ((GPIO_Init->Pull) << (position * 2));
  GPIOB->PUPDR = temp;

  /* Configure IO Direction mode (Input, Output, Alternate or Analog) */
  temp = GPIOB->MODER;
  temp &= ~(GPIO_MODER_MODER0 << (position * 2));
  temp |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2));
  GPIOB->MODER = temp;
}


// this call would be inlined as a direct bitwise access
// thus zero overhead in calling & parameterization
void HAL_GPIO_WritePin(gpiox, pin_number, state)
{
  gpiox->ODR = state << pin_number;
}

// can use the BSRR register for _Set and _Reset (bit-shift determines Set or Reset)
// can use a READ_REG / WRITE_REG pair to perform toggle
