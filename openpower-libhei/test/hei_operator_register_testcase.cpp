#include <hei_chip.hpp>
#include <hei_includes.hpp>
#include <register/hei_hardware_register.hpp>
#include <register/hei_operator_register.hpp>
#include <register/hei_register.hpp>

#include <iomanip>
#include <iostream>

#define BITSTRING_BIT_WIDTH 16
#define CONSTANT1 0x1458 // 1458
#define CONSTANT2 0x5368 // 5468

using namespace std;
using namespace libhei;

int main()
{

    Chip my_chip{nullptr, 0};
    size_t constant1 = CONSTANT1;
    size_t constant2 = CONSTANT2;

    // Initializing two registers

    cout << "\n\n\nInitializing two constant registers ";
    cout << "(leftside & rightside):\n\n";

    cout << "\nCreating BitString \"bsa\".\n";
    BitStringBuffer bsa{BITSTRING_BIT_WIDTH};
    cout << "Setting BitString \"bsa\" to:  ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << uppercase << constant1 << "\n";
    bsa.setFieldRight(0, BITSTRING_BIT_WIDTH, constant1);

    cout << "\nCreating BitString \"bsb\".\n";
    BitStringBuffer bsb{BITSTRING_BIT_WIDTH};
    cout << "Setting BitString \"bsb\" to:  ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << uppercase << constant2 << "\n\n";
    bsb.setFieldRight(0, BITSTRING_BIT_WIDTH, constant2);

    cout << "Creating \"leftside\" register from bsa.\n";
    ConstantRegister leftside(bsa);
    cout << "Creating \"rightside\" register from bsb.\n";
    ConstantRegister rightside(bsb);

    cout << "Leftside:  ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << leftside.getBitString(my_chip)->getFieldRight(0,
                                                          BITSTRING_BIT_WIDTH);

    cout << "  Rightside:  ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << rightside.getBitString(my_chip)->getFieldRight(0,
                                                           BITSTRING_BIT_WIDTH);
    cout << "\n\n";

    // Basic functional tests:
    // AND
    cout << "Instantiating my_and_register passing in two registers:\n";
    cout << "AndRegister my_and_register(leftside, rightside);\n";
    AndRegister my_and_register(leftside, rightside);
    cout << "AND result:  ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << my_and_register.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n\n";

    // NOT
    cout << "Instantiating my_not_register passing in my_and_register "
         << "from above:\n";
    cout << "NotRegister my_not_register(my_and_register);\n";
    NotRegister my_not_register(my_and_register);
    cout << "NOT result:               ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << my_not_register.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n\n";

    // Left Shift
    cout << "Instantiating my_leftshift_register passing "
         << "in my_not_register from above and a 4 directing i"
         << "a left shift of 4 bits:\n";
    cout << "LeftShiftRegister my_leftshift_register(my_not_register, 4);\n";
    LeftShiftRegister my_leftshift_register(my_not_register, 4);
    cout << "LeftShift result:         ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << my_leftshift_register.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n\n";

    // Right Shift
    cout << "Instantiating my_rightshift_register passing in "
         << "my_leftshift_register from above and a 4 directing "
         << "a right shift of 4 bits:\n";
    cout << "RightShiftRegister my_rightshift_register("
         << "my_leftshift_register, 4);\n";
    RightShiftRegister my_rightshift_register(my_leftshift_register, 4);
    cout << "Right Shift result:         ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << my_rightshift_register.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n\n";

    // OR
    cout << "Instantiating my_or_register passing in my_leftshift_register"
         << " and my_rightshift_register from above:\n";
    cout << "OrRegister my_or_register(my_leftshift_register,"
         << "my_rightshift_register);\n";
    OrRegister my_or_register(my_leftshift_register, my_rightshift_register);
    cout << "OR result:         ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << my_or_register.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n\n";

    // = operator

    // ConstantRegister
    cout << "\nExercising operator \"=\" (assignment):\n\n";

    cout << "Assigning leftside register to constreg1 register.  (";
    ConstantRegister constreg1 = leftside;
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << constreg1.getBitString(my_chip)->getFieldRight(0,
                                                           BITSTRING_BIT_WIDTH)
         << ")\n";
    cout << "ConstantRegister constreg1 = leftside;\n";

    cout << "Assigning rightside register to constreg2 register.  (";
    ConstantRegister constreg2 = rightside;
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << constreg2.getBitString(my_chip)->getFieldRight(0,
                                                           BITSTRING_BIT_WIDTH)
         << ")\n";
    cout << "ConstantRegister constreg2 = rightside;\n";

    // AndRegister
    cout << "\nCreating and_register1 with leftside & rightside registers.\n";
    cout << "AndRegister and_register1(leftside,rightside);\n";
    AndRegister and_register1(leftside, rightside);
    cout << "Creating and_register2 with the assignment operator (=).\n";
    cout << "AndRegister and_register2 = and_register1;\n";
    AndRegister and_register2 = and_register1;
    cout << "and_register2 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << and_register2.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";

    // NotRegister
    cout << "\nCreating not_register1 with leftside register.\n";
    cout << "NotRegister not_register1(leftside);\n";
    NotRegister not_register1(leftside);
    cout << "Creating not_register2 with the assignment operator (=).\n";
    cout << "NotRegister not_register2 = not_register1;\n";
    NotRegister not_register2 = not_register1;
    cout << "not_register2 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << not_register2.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";

    // LeftShiftRegister
    cout << "\nCreating leftshift_register1 with leftside register.\n";
    cout << "LeftShiftRegister leftshift_register1(leftside,4);\n";
    LeftShiftRegister leftshift_register1(leftside, 4);
    cout << "Creating leftshift_register2 with the assignment operator (=).\n";
    cout << "LeftShiftRegister leftshift_register2 = leftshift_register1;\n";
    LeftShiftRegister leftshift_register2 = leftshift_register1;
    cout << "leftshift_register2 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << leftshift_register2.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";

    // RightShiftRegister
    cout << "\nCreating rightshift_register1 with leftside register.\n";
    cout << "RightShiftRegister rightshift_register1(leftside,4);\n";
    RightShiftRegister rightshift_register1(leftside, 4);
    cout << "Creating rightshift_register2 with the assignment operator (=).\n";
    cout << "RightShiftRegister rightshift_register2 = rightshift_register1;\n";
    RightShiftRegister rightshift_register2 = rightshift_register1;
    cout << "rightshift_register2 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << rightshift_register2.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";

    // OrRegister
    cout << "\nCreating or_register1 with leftside & rightside registers.\n";
    cout << "OrRegister or_register1(leftside,rightside);\n";
    OrRegister or_register1(leftside, rightside);
    cout << "or_register1 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << or_register1.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";
    cout << "Creating or_register2 with the assignment operator (=).\n";
    cout << "OrRegister or_register2 = or_register1;\n";
    OrRegister or_register2 = or_register1;
    cout << "or_register2 = ";
    cout << "0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << or_register2.getBitString(my_chip)->getFieldRight(
                0, BITSTRING_BIT_WIDTH)
         << "\n";

    //***** == operator *****
    cout << "\nOperator \"==\" (equality)\n\n";
    cout << "Operation  Register1 = Register2    Equality\n";

    // Constant Register
    ConstantRegister cr1(leftside);
    ConstantRegister cr2(rightside);
    cout << " Constant    0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << cr1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << cr2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (cr1 == cr2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    // And Register
    AndRegister and1(leftside, rightside);
    AndRegister and2(leftside, rightside);

    cout << " And         0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << and1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << and2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (and1 == and2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    // Not Register
    NotRegister nr1(leftside);
    NotRegister nr2(rightside);
    cout << " Not         0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << nr1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << nr2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (nr1 == nr2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    // Left Shift Register
    LeftShiftRegister lsr1(leftside, 4);
    LeftShiftRegister lsr2(rightside, 4);
    cout << " Left Shift  0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << lsr1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << lsr2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (lsr1 == lsr2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    // Right Shift Register
    RightShiftRegister rsr1(leftside, 6);
    RightShiftRegister rsr2(rightside, 6);
    cout << " Right Shift 0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << rsr1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << rsr2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (rsr1 == rsr2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    // Or Register
    OrRegister or1(leftside, rightside);
    OrRegister or2(leftside, rightside);
    cout << " OR          0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << or1.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << or2.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    if (or1 == or2)
        cout << "      Equal\n";
    else
        cout << "      Not Equal\n";

    //***** < operator *****
    cout << "\nOperator \"<\" (equality)\n\n";
    cout << "Operation  Register1 < Register2  Equal/Less Than\n";

    // Constant Register
    ConstantRegister cr1a(leftside);
    ConstantRegister cr2a(rightside);
    cout << " Constant    0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << cr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << cr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (cr1a < cr2a)
    if (cr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        cr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    // And Register
    AndRegister and1a(leftside, rightside);
    AndRegister and2a(leftside, rightside);
    cout << " And         0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << and1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << and2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (and2a < and1a)
    if (and1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        and2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    // Not Register
    NotRegister nr1a(leftside);
    NotRegister nr2a(rightside);
    cout << " Not         0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << nr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << nr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (nr1a < nr2a) Paul
    if (nr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        nr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    // Left Shift Register
    LeftShiftRegister lsr1a(leftside, 4);
    LeftShiftRegister lsr2a(leftside, 4);
    cout << " Left Shift  0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << lsr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << lsr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (lsr1a < lsr2a)
    if (lsr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        lsr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    // Right Shift Register
    RightShiftRegister rsr1a(leftside, 4);
    RightShiftRegister rsr2a(leftside, 4);
    cout << " Right Shift 0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << rsr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << rsr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (rsr1a < rsr2a)
    if (rsr1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        rsr2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    // Or Register
    OrRegister or1a(leftside, rightside);
    OrRegister or2a(leftside, rightside);
    cout << " Or          0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4)
         << hex
         << or1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    cout << "     0x" << setfill('0') << setw(BITSTRING_BIT_WIDTH / 4) << hex
         << or2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH);
    // if (or1a < or2a)
    if (or1a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH) <
        or2a.getBitString(my_chip)->getFieldRight(0, BITSTRING_BIT_WIDTH))
        cout << "      Less Than\n";
    else
        cout << "      Not Less Than\n";

    return (0);
}
