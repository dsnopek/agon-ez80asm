#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "instruction.h"
#include "assemble.h"
#include "utils.h"
#include "globals.h"
#include "./stdint.h"

bool none_match(operand *op) {
    return ((op->reg == R_NONE) && (op->immediate_provided == false) & !(op->cc));
}
bool cc_match(operand *op) {
    return op->cc;
}
bool ir_match(operand *op) {
    return ((op->reg >= R_IXH) && (op->reg <= R_IYL) && !(op->indirect));
}
bool ixy_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect)  && !(op->displacement_provided));
}
bool ixyd_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && !(op->indirect) && (op->displacement_provided));
}
bool indirect_ixyd_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && op->indirect);
}
bool mmn_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool indirect_mmn_match(operand *op) {
    return ((op->indirect) && (op->immediate_provided));
}
bool n_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool a_match(operand *op) {
    return(op->reg == R_A);
}
bool hl_match(operand *op) {
    return((op->reg == R_HL) && !(op->indirect));
}
bool indirect_hl_match(operand *op) {
    return((op->reg == R_HL) && (op->indirect));
}
bool rr_match(operand *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && !(op->indirect));
}
bool indirect_rr_match(operand *op) {
    return((op->reg >= R_BC) && (op->reg <= R_HL) && (op->indirect));
}
bool rxy_match(operand *op) {
    return(!(op->indirect) && ((op->reg == R_BC) || (op->reg == R_DE) || (op->reg == R_IX) || (op->reg == R_IY)));
}
bool sp_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_SP));
}
bool indirect_sp_match(operand *op) {
    return((op->indirect) && (op->reg == R_SP));
}
bool r_match(operand *op) {
    return((op->reg >= R_A) && (op->reg <= R_L));
}
bool reg_r_match(operand *op) {
    return(op->reg == R_R);
}
bool mb_match(operand *op) {
    return(op->reg == R_MB);
}
bool i_match(operand *op) {
    return(op->reg == R_I);
}
bool b_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool af_match(operand *op) {
    return(op->reg == R_AF);
}
bool de_match(operand *op) {
    return(op->reg == R_DE);
}
bool nselect_match(operand *op) {
    return (!(op->indirect) && (op->immediate_provided));
}
bool indirect_n_match(operand *op) {
    return ((op->indirect) && (op->immediate_provided));
}
bool indirect_bc_match(operand *op) {
    return((op->indirect) && (op->reg == R_BC));
}
bool indirect_c_match(operand *op) {
    return((op->indirect) && (op->reg == R_C));
}
bool indirect_ixy_match(operand *op) {
    return (((op->reg == R_IX) || (op->reg == R_IY)) && (op->indirect)  && !(op->displacement_provided));
}
bool cca_match(operand *op) {
    return ((op->cc) &&
            ((op->cc_index == CC_INDEX_NZ) ||
             (op->cc_index == CC_INDEX_Z) ||
             (op->cc_index == CC_INDEX_NC) ||
             (op->cc_index == CC_INDEX_C)));
}
bool indirect_de_match(operand *op) {
    return((op->indirect) && (op->reg == R_DE));
}
bool ix_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_IX));
}
bool iy_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_IY));
}
bool ixd_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_IX));
}
bool iyd_match(operand *op) {
    return(!(op->indirect) && (op->reg == R_IY));
}
bool indirect_ixd_match(operand *op) {
    return((op->indirect) && (op->reg == R_IX));
}
bool indirect_iyd_match(operand *op) {
    return((op->indirect) && (op->reg == R_IY));
}
bool raeonly_match(operand *op) {
    return((op->reg >= R_A) && (op->reg <= R_E));
}

// table with fast access to functions that perform matching to an specific permittype
permittype_match permittype_matchlist[] = {
    {OPTYPE_NONE,           none_match},
    {OPTYPE_CC,             cc_match},
    {OPTYPE_IR,             ir_match},
    {OPTYPE_IXY,            ixy_match},
    {OPTYPE_IXYd,           ixyd_match},
    {OPTYPE_INDIRECT_IXYd,  indirect_ixyd_match},
    {OPTYPE_MMN,            mmn_match},
    {OPTYPE_INDIRECT_MMN,   indirect_mmn_match},
    {OPTYPE_N,              n_match},
    {OPTYPE_A,              a_match},
    {OPTYPE_HL,             hl_match},
    {OPTYPE_INDIRECT_HL,    indirect_hl_match},
    {OPTYPE_RR,             rr_match},
    {OPTYPE_INDIRECT_RR,    indirect_rr_match},
    {OPTYPE_RXY,            rxy_match},
    {OPTYPE_SP,             sp_match},
    {OPTYPE_INDIRECT_SP,    indirect_sp_match},
    {OPTYPE_R,              r_match},
    {OPTYPE_REG_R,          reg_r_match},
    {OPTYPE_MB,             mb_match},
    {OPTYPE_I,              i_match},
    {OPTYPE_BIT,            b_match},
    {OPTYPE_AF,             af_match},
    {OPTYPE_DE,             de_match},
    {OPTYPE_NSELECT,        nselect_match},
    {OPTYPE_INDIRECT_N,     indirect_n_match},
    {OPTYPE_INDIRECT_BC,    indirect_bc_match},
    {OPTYPE_INDIRECT_C,     indirect_c_match},
    {OPTYPE_INDIRECT_IXY,   indirect_ixy_match},
    {OPTYPE_CCA,            cca_match},
    {OPTYPE_INDIRECT_DE,    indirect_de_match},
    {OPTYPE_IX,             ix_match},
    {OPTYPE_IY,             iy_match},
    {OPTYPE_IXd,            ixd_match},
    {OPTYPE_IYd,            iyd_match},
    {OPTYPE_INDIRECT_IXd,   indirect_ixd_match},
    {OPTYPE_INDIRECT_IYd,   indirect_iyd_match},
    {OPTYPE_R_AEONLY,       raeonly_match}
};

operandlist operands_adc[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x8C, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x8E, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x88, S_NONE},
    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x4A, S_S1L0},
    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7A, S_S1L0},
};
operandlist operands_add[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x84, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x86, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x80, S_NONE},
    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_S1L0},
    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_S1L0},
    {OPTYPE_IXY, OPTYPE_RXY,             true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x09, S_S1L0},
    {OPTYPE_IXY, OPTYPE_SP,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x39, S_S1L0},
};
operandlist operands_and[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xA4, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA0, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xA4, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xA6, S_S1L0},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE6, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA0, S_NONE},
};
operandlist operands_bit[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_S1L0},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x46, S_S1L0},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_Y,      TRANSFORM_Z,    0xCB, 0x40, S_NONE},
};
operandlist operands_call[] = {
    {OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC4, S_ANY},
    {OPTYPE_MMN, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xCD, S_ANY},
};
operandlist operands_ccf[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_NONE},
};
operandlist operands_cp[]= {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xBC, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB8, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_S1L0},
    {OPTYPE_IR,  OPTYPE_NONE,            true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xBC, S_NONE},
    {OPTYPE_INDIRECT_IXYd,  OPTYPE_NONE, true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xBE, S_S1L0},
    {OPTYPE_N,  OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFE, S_NONE},
    {OPTYPE_R,  OPTYPE_NONE,            false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB8, S_NONE},

};
operandlist operands_cpd[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA9, S_S1L0},
};
operandlist operands_cpdr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB9, S_S1L0},
};
operandlist operands_cpi[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA1, S_S1L0},
};
operandlist operands_cpir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB1, S_S1L0},
};
operandlist operands_cpl[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2F, S_NONE},
};
operandlist operands_daa[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x27, S_NONE},
};
operandlist operands_dec[]= {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x25, S_NONE},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2B, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x35, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x05, S_NONE},
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x0B, S_S1L0},
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3B, S_S1L0},
};
operandlist operands_di[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF3, S_NONE},
};
operandlist operands_djnz[]= {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x10, S_NONE},
};
operandlist operands_ei[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xFB, S_NONE},
};
operandlist operands_ex[]= {
    {OPTYPE_AF, OPTYPE_AF,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x08, S_NONE},
    {OPTYPE_DE, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEB, S_NONE},
    {OPTYPE_INDIRECT_SP, OPTYPE_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_S1L0},
    {OPTYPE_INDIRECT_SP, OPTYPE_IXY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE3, S_S1L0},
};
operandlist operands_exx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD9, S_NONE},
};
operandlist operands_halt[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x76, S_NONE},
};
operandlist operands_im[]= {
    {OPTYPE_NSELECT, OPTYPE_NONE,       false, TRANSFORM_SELECT, TRANSFORM_NONE, 0xED, 0x46, S_NONE},
};
operandlist operands_in[]= {
    {OPTYPE_A, OPTYPE_INDIRECT_N,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDB, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_BC,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_C,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x40, S_NONE},
};
operandlist operands_in0[]= {
    {OPTYPE_R, OPTYPE_INDIRECT_N,       false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x00, S_NONE},
};
operandlist operands_inc[]= {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x24, S_NONE},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x23, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x34, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x04, S_NONE},
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x03, S_S1L0},
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x33, S_S1L0},
};
operandlist operands_ind[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAA, S_S1L0},
};
operandlist operands_ind2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8C, S_S1L0},
};
operandlist operands_ind2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9C, S_S1L0},
};
operandlist operands_indm[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8A, S_S1L0},
};
operandlist operands_indmr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9A, S_S1L0},
};
operandlist operands_indr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBA, S_S1L0},
};
operandlist operands_indrx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCA, S_S1L0},
};
operandlist operands_ini[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA2, S_S1L0},
};
operandlist operands_ini2[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x84, S_S1L0},
};
operandlist operands_ini2r[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x94, S_S1L0},
};
operandlist operands_inim[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x82, S_S1L0},
};
operandlist operands_inimr[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x92, S_S1L0},
};
operandlist operands_inir[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB2, S_S1L0},
};
operandlist operands_inirx[]= {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC2, S_S1L0},
};
operandlist operands_jp[] = {
    {OPTYPE_CC, OPTYPE_MMN,             false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC2, S_SISLIL},
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_S1L0},
    {OPTYPE_INDIRECT_IXY, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE9, S_SISLIL},
    {OPTYPE_MMN, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC3, S_SISLIL},
};
operandlist operands_jr[]= {
    {OPTYPE_CCA, OPTYPE_N,              false, TRANSFORM_CC,     TRANSFORM_REL,  0x00, 0x20, S_NONE},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_REL,    TRANSFORM_NONE, 0x00, 0x18, S_NONE},
};
operandlist operands_ld[] = {
    {OPTYPE_A, OPTYPE_I,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x57, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_S1L0},
    {OPTYPE_A, OPTYPE_MB,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6E, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_MMN,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3A, S_SISLIL},
    {OPTYPE_A, OPTYPE_REG_R,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x5F, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_BC,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0A, S_S1L0},
    {OPTYPE_A, OPTYPE_INDIRECT_DE,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1A, S_S1L0},
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x7E, S_S1L0},
    {OPTYPE_HL, OPTYPE_I,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xD7, S_NONE},
    {OPTYPE_INDIRECT_HL, OPTYPE_IX,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3F, S_S1L0},
    {OPTYPE_INDIRECT_HL, OPTYPE_IY,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x3E, S_S1L0},
    {OPTYPE_INDIRECT_HL, OPTYPE_N,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_S1L0},
    {OPTYPE_INDIRECT_HL, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_S1L0},
    {OPTYPE_INDIRECT_HL, OPTYPE_RR,     false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x0F, S_S1L0},
    {OPTYPE_I, OPTYPE_HL,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC7, S_NONE},
    {OPTYPE_I, OPTYPE_A,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x47, S_NONE},
    {OPTYPE_IR, OPTYPE_IR,               true, TRANSFORM_IR3,     TRANSFORM_IR0, 0x00, 0x64, S_NONE},
    {OPTYPE_IR, OPTYPE_N,                true, TRANSFORM_IR3,     TRANSFORM_NONE,0x00, 0x26, S_NONE},
    {OPTYPE_IR, OPTYPE_R_AEONLY,         true, TRANSFORM_IR3,     TRANSFORM_Z,   0x00, 0x60, S_NONE},
    {OPTYPE_IX, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x37, S_S1L0},
    {OPTYPE_IY, OPTYPE_INDIRECT_HL,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x31, S_S1L0},
    {OPTYPE_IX, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_S1L0},
    {OPTYPE_IY, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_S1L0},
    {OPTYPE_IX, OPTYPE_INDIRECT_IYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_S1L0},
    {OPTYPE_IY, OPTYPE_INDIRECT_IXd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_S1L0},
    {OPTYPE_IXY, OPTYPE_MMN,             true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x21, S_SISLIL},
    {OPTYPE_IXY, OPTYPE_INDIRECT_MMN,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_SISLIL},
    {OPTYPE_INDIRECT_IXd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_S1L0},
    {OPTYPE_INDIRECT_IYd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3F, S_S1L0},
    {OPTYPE_INDIRECT_IXd, OPTYPE_IY,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_S1L0},
    {OPTYPE_INDIRECT_IYd, OPTYPE_IX,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x3E, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_N,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x36, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_R,     true, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x70, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_RR,    true, TRANSFORM_NONE,   TRANSFORM_P,    0x00, 0x0F, S_S1L0},
    {OPTYPE_MB, OPTYPE_A,               false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6D, S_NONE},
    {OPTYPE_INDIRECT_MMN, OPTYPE_A,     false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x32, S_SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_IXY,    true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_HL,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x22, S_SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_RR,    false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x43, S_SISLIL},
    {OPTYPE_INDIRECT_MMN, OPTYPE_SP,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x73, S_SISLIL},
    {OPTYPE_REG_R, OPTYPE_A,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4F, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_HL,      false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_S1L0},
    {OPTYPE_R_AEONLY, OPTYPE_IR,         true, TRANSFORM_Y,      TRANSFORM_IR0,  0x00, 0x44, S_NONE},
    {OPTYPE_R, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x46, S_S1L0},
    {OPTYPE_R, OPTYPE_N,                false, TRANSFORM_Y,      TRANSFORM_NONE, 0x00, 0x06, S_NONE},
    {OPTYPE_R, OPTYPE_R,                false, TRANSFORM_Y,      TRANSFORM_Z,    0x00, 0x40, S_NONE},
    {OPTYPE_RR, OPTYPE_INDIRECT_HL,     false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x07, S_S1L0},
    {OPTYPE_RR, OPTYPE_INDIRECT_IXYd,    true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x07, S_S1L0},
    {OPTYPE_RR, OPTYPE_MMN,              true, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x01, S_SISLIL},
    {OPTYPE_HL, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x2A, S_SISLIL},
    {OPTYPE_RR, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4B, S_SISLIL},
    {OPTYPE_INDIRECT_HL, OPTYPE_A,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x77, S_S1L0},
    {OPTYPE_INDIRECT_RR, OPTYPE_A,      false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0x02, S_S1L0},
    {OPTYPE_SP, OPTYPE_HL,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_S1L0},
    {OPTYPE_SP, OPTYPE_IXY,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF9, S_S1L0},
    {OPTYPE_SP, OPTYPE_MMN,              true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x31, S_SISLIL},
    {OPTYPE_SP, OPTYPE_INDIRECT_MMN,    false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7B, S_SISLIL},
};
operandlist operands_ldd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA8, S_S1L0},
};
operandlist operands_lddr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB8, S_S1L0},
};
operandlist operands_ldi[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA0, S_S1L0},
};
operandlist operands_ldir[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB0, S_S1L0},
};
operandlist operands_lea[] = {
    {OPTYPE_IX, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x32, S_S1L0},
    {OPTYPE_IY, OPTYPE_IXd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x55, S_S1L0},
    {OPTYPE_IX, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x54, S_S1L0},
    {OPTYPE_IY, OPTYPE_IYd,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x33, S_S1L0},
    {OPTYPE_RR, OPTYPE_IXd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x02, S_S1L0},
    {OPTYPE_RR, OPTYPE_IYd,             false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x03, S_S1L0},
};
operandlist operands_mlt[] = {
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0xED, 0x4C, S_NONE},
    {OPTYPE_SP, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7C, S_S1L0},
};
operandlist operands_neg[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x44, S_NONE},
};
operandlist operands_nop[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x00, S_NONE},
};
operandlist operands_or[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xB4, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xB0, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xB4, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xB6, S_S1L0},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF6, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xB0, S_NONE},
};
operandlist operands_otd2r[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBC, S_S1L0},
};
operandlist operands_otdm[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x8B, S_S1L0},
};
operandlist operands_otdmr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x9B, S_S1L0},
};
operandlist operands_otdr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xBB, S_S1L0},
};
operandlist operands_otdrx[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xCB, S_S1L0},
};
operandlist operands_oti2r[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB4, S_S1L0},
};
operandlist operands_otim[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x83, S_S1L0},
};
operandlist operands_otimr[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x93, S_S1L0},
};
operandlist operands_otir[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xB3, S_S1L0},
};
operandlist operands_otirx[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xC3, S_S1L0},
};
operandlist operands_out[] = {
    {OPTYPE_INDIRECT_BC, OPTYPE_R,      false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {OPTYPE_INDIRECT_C, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x41, S_NONE},
    {OPTYPE_INDIRECT_N, OPTYPE_A,       false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD3, S_NONE},
};
operandlist operands_out0[] = {
    {OPTYPE_INDIRECT_N, OPTYPE_R,       false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x01, S_NONE},
};
operandlist operands_outd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAB, S_S1L0},
};
operandlist operands_outd2[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xAC, S_S1L0},
};
operandlist operands_outi[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA3, S_S1L0},
};
operandlist operands_outi2[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0xA4, S_S1L0},
};
operandlist operands_pea[] = {
    {OPTYPE_IXd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x65, S_S1L0},
    {OPTYPE_IYd, OPTYPE_NONE,           false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x66, S_S1L0},
};
operandlist operands_pop[] = {
    {OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF1, S_S1L0},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE1, S_S1L0},
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC1, S_S1L0},
};
operandlist operands_push[] = {
    {OPTYPE_AF, OPTYPE_NONE,            false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xF5, S_S1L0},
    {OPTYPE_IXY, OPTYPE_NONE,            true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xE5, S_S1L0},
    {OPTYPE_RR, OPTYPE_NONE,            false, TRANSFORM_P,      TRANSFORM_NONE, 0x00, 0xC5, S_S1L0},
};
operandlist operands_res[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_S1L0},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0x86, S_S1L0},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0x80, S_NONE},
};
operandlist operands_ret[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xC9, S_LIL | S_LIS},
    {OPTYPE_CC, OPTYPE_NONE,            false, TRANSFORM_CC,     TRANSFORM_NONE, 0x00, 0xC0, S_LIL | S_LIS},
};
operandlist operands_reti[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x4D, S_LIL | S_LIS},
};
operandlist operands_retn[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x45, S_LIL | S_LIS},
};
operandlist operands_rl[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x16, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x10, S_NONE},
};
operandlist operands_rla[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x17, S_NONE},
};
operandlist operands_rlc[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x06, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,     TRANSFORM_NONE,  0xCB, 0x00, S_NONE},
};
operandlist operands_rlca[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x07, S_NONE},
};
operandlist operands_rld[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x6F, S_NONE},
};
operandlist operands_rr[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x1E, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x18, S_NONE},
};
operandlist operands_rra[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x1F, S_NONE},
};
operandlist operands_rrc[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x0E, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x08, S_NONE},
};
operandlist operands_rrca[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x0F, S_NONE},
};
operandlist operands_rrd[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x67, S_NONE},
};
operandlist operands_rsmix[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7E, S_NONE},
};
operandlist operands_rst[] = {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_N,      TRANSFORM_NONE, 0x00, 0xC7, S_SISLIL},
};
operandlist operands_sbc[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x9C, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x9E, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xDE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x98, S_NONE},
    {OPTYPE_HL, OPTYPE_RR,              false, TRANSFORM_NONE,   TRANSFORM_P,    0xED, 0x42, S_S1L0},
    {OPTYPE_HL, OPTYPE_SP,              false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x72, S_S1L0},
};
operandlist operands_scf[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x37, S_NONE},
};
operandlist operands_set[] = {
    {OPTYPE_BIT, OPTYPE_INDIRECT_HL,    false, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_S1L0},
    {OPTYPE_BIT, OPTYPE_INDIRECT_IXYd,   true, TRANSFORM_Y,      TRANSFORM_NONE, 0xCB, 0xC6, S_S1L0},
    {OPTYPE_BIT, OPTYPE_R,              false, TRANSFORM_BIT,    TRANSFORM_Z,    0xCB, 0xC0, S_NONE},
};
operandlist operands_sla[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x26, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x20, S_NONE},
};
operandlist operands_slp[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x76, S_NONE},
};
operandlist operands_sra[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x2E, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x28, S_NONE},
};
operandlist operands_srl[] = {
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_S1L0},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0xCB, 0x3E, S_S1L0},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0xCB, 0x38, S_NONE},
};
operandlist operands_stmix[] = {
    {OPTYPE_NONE, OPTYPE_NONE,          false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x7D, S_NONE},
};
operandlist operands_sub[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0x94, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0x96, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xD6, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0x90, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,    false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,              true, TRANSFORM_IR0,    TRANSFORM_NONE,0x00, 0x94, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,   true, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0x96, S_S1L0},
    {OPTYPE_N, OPTYPE_NONE,              false, TRANSFORM_NONE,   TRANSFORM_NONE,0x00, 0xD6, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,              false, TRANSFORM_Z,      TRANSFORM_NONE,0x00, 0x90, S_NONE},
};
operandlist operands_tst[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Y,    0xED, 0x04, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x34, S_S1L0},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x64, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Y,      TRANSFORM_NONE, 0xED, 0x04, S_NONE},
};
operandlist operands_tstio[] = {
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0xED, 0x74, S_NONE},
};
operandlist operands_xor[] = {
    {OPTYPE_A, OPTYPE_INDIRECT_HL,      false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_S1L0},
    {OPTYPE_A, OPTYPE_IR,                true, TRANSFORM_NONE,   TRANSFORM_IR0,  0x00, 0xAC, S_NONE},
    {OPTYPE_A, OPTYPE_INDIRECT_IXYd,     true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_S1L0},
    {OPTYPE_A, OPTYPE_N,                false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {OPTYPE_A, OPTYPE_R,                false, TRANSFORM_NONE,   TRANSFORM_Z,    0x00, 0xA8, S_NONE},
    // same set, without A register
    {OPTYPE_INDIRECT_HL, OPTYPE_NONE,   false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_S1L0},
    {OPTYPE_IR, OPTYPE_NONE,             true, TRANSFORM_IR0,    TRANSFORM_NONE, 0x00, 0xAC, S_NONE},
    {OPTYPE_INDIRECT_IXYd, OPTYPE_NONE,  true, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xAE, S_S1L0},
    {OPTYPE_N, OPTYPE_NONE,             false, TRANSFORM_NONE,   TRANSFORM_NONE, 0x00, 0xEE, S_NONE},
    {OPTYPE_R, OPTYPE_NONE,             false, TRANSFORM_Z,      TRANSFORM_NONE, 0x00, 0xA8, S_NONE},
};


// this table needs to be sorted on name
instruction instructions[] = {
    {"adc",      EZ80, 0, sizeof(operands_adc)/sizeof(operandlist), operands_adc},
    {"add",      EZ80, 0, sizeof(operands_add)/sizeof(operandlist), operands_add},
    {"align",    ASSEMBLER, ASM_ALIGN, 0, NULL, ASM_ARG_SINGLE},
    {"and",      EZ80, 0, sizeof(operands_and)/sizeof(operandlist), operands_and},
    {"ascii",    ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"asciz",    ASSEMBLER, ASM_ASCIZ, 0, NULL, ASM_ARG_LIST},
    {"assume",   ASSEMBLER, ASM_ADL, 0, NULL, ASM_ARG_KEYVAL},
    {"bit",      EZ80, 0, sizeof(operands_bit)/sizeof(operandlist), operands_bit},
    {"blkb",     ASSEMBLER, ASM_BLKB, 0, NULL, ASM_ARG_LIST},
    {"blkl",     ASSEMBLER, ASM_BLKL, 0, NULL, ASM_ARG_LIST},
    {"blkp",     ASSEMBLER, ASM_BLKP, 0, NULL, ASM_ARG_LIST},
    {"blkw",     ASSEMBLER, ASM_BLKW, 0, NULL, ASM_ARG_LIST},
    {"byte",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"call",     EZ80, 0, sizeof(operands_call)/sizeof(operandlist), operands_call},
    {"ccf",      EZ80, 0, sizeof(operands_ccf)/sizeof(operandlist), operands_ccf},
    {"cp",       EZ80, 0, sizeof(operands_cp)/sizeof(operandlist), operands_cp},
    {"cpd",      EZ80, 0, sizeof(operands_cpd)/sizeof(operandlist), operands_cpd},
    {"cpdr",     EZ80, 0, sizeof(operands_cpdr)/sizeof(operandlist), operands_cpdr},
    {"cpi",      EZ80, 0, sizeof(operands_cpi)/sizeof(operandlist), operands_cpi},
    {"cpir",     EZ80, 0, sizeof(operands_cpir)/sizeof(operandlist), operands_cpir},
    {"cpl",      EZ80, 0, sizeof(operands_cpl)/sizeof(operandlist), operands_cpl},
    {"daa",      EZ80, 0, sizeof(operands_daa)/sizeof(operandlist), operands_daa},
    {"db",       ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"dec",      EZ80, 0, sizeof(operands_dec)/sizeof(operandlist), operands_dec},
    {"defb",     ASSEMBLER, ASM_DB, 0, NULL, ASM_ARG_LIST},
    {"define",   ASSEMBLER, ASM_DEFINE, 0, NULL, ASM_ARG_SINGLE},
    {"defs",     ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"defw",     ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"di",       EZ80, 0, sizeof(operands_di)/sizeof(operandlist), operands_di},
    {"djnz",     EZ80, 0, sizeof(operands_djnz)/sizeof(operandlist), operands_djnz},
    {"dl",       ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"ds",       ASSEMBLER, ASM_DS, 0, NULL, ASM_ARG_LIST},
    {"dw",       ASSEMBLER, ASM_DW, 0, NULL, ASM_ARG_LIST},
    {"dw24",     ASSEMBLER, ASM_DW24, 0, NULL, ASM_ARG_LIST},
    {"ei",       EZ80, 0, sizeof(operands_ei)/sizeof(operandlist), operands_ei},
    {"else",     ASSEMBLER, ASM_ELSE, 0, NULL, ASM_ARG_NONE},
    {"endif",    ASSEMBLER, ASM_ENDIF, 0, NULL, ASM_ARG_NONE},
    {ENDMACROCMD,ASSEMBLER, ASM_MACRO_END, 0, NULL, ASM_ARG_SINGLE},
    {"equ",      ASSEMBLER, ASM_EQU, 0, NULL, ASM_ARG_SINGLE},
    {"ex",       EZ80, 0, sizeof(operands_ex)/sizeof(operandlist), operands_ex},
    {"exx",      EZ80, 0, sizeof(operands_exx)/sizeof(operandlist), operands_exx},
    {"halt",     EZ80, 0, sizeof(operands_halt)/sizeof(operandlist), operands_halt},
    {"ifdef",    ASSEMBLER, ASM_IFDEF, 0, NULL, ASM_ARG_SINGLE},
    {"im",       EZ80, 0, sizeof(operands_im)/sizeof(operandlist), operands_im},
    {"in",       EZ80, 0, sizeof(operands_in)/sizeof(operandlist), operands_in},
    {"in0",      EZ80, 0, sizeof(operands_in0)/sizeof(operandlist), operands_in0},
    {"inc",      EZ80, 0, sizeof(operands_inc)/sizeof(operandlist), operands_inc},
    {"incbin",   ASSEMBLER, ASM_INCBIN, 0, NULL, ASM_ARG_SINGLE},
    {"include",  ASSEMBLER, ASM_INCLUDE, 0, NULL, ASM_ARG_SINGLE},
    {"ind",      EZ80, 0, sizeof(operands_ind)/sizeof(operandlist), operands_ind},
    {"ind2",     EZ80, 0, sizeof(operands_ind2)/sizeof(operandlist), operands_ind2},
    {"ind2r",    EZ80, 0, sizeof(operands_ind2r)/sizeof(operandlist), operands_ind2r},
    {"indm",     EZ80, 0, sizeof(operands_indm)/sizeof(operandlist), operands_indm},
    {"indmr",    EZ80, 0, sizeof(operands_indmr)/sizeof(operandlist), operands_indmr},
    {"indr",     EZ80, 0, sizeof(operands_indr)/sizeof(operandlist), operands_indr},
    {"indrx",    EZ80, 0, sizeof(operands_indrx)/sizeof(operandlist), operands_indrx},
    {"ini",      EZ80, 0, sizeof(operands_ini)/sizeof(operandlist), operands_ini},
    {"ini2",     EZ80, 0, sizeof(operands_ini2)/sizeof(operandlist), operands_ini2},
    {"ini2r",    EZ80, 0, sizeof(operands_ini2r)/sizeof(operandlist), operands_ini2r},
    {"inim",     EZ80, 0, sizeof(operands_inim)/sizeof(operandlist), operands_inim},
    {"inimr",    EZ80, 0, sizeof(operands_inimr)/sizeof(operandlist), operands_inimr},
    {"inir",     EZ80, 0, sizeof(operands_inir)/sizeof(operandlist), operands_inir},
    {"inirx",    EZ80, 0, sizeof(operands_inirx)/sizeof(operandlist), operands_inirx},
    {"jp",       EZ80, 0, sizeof(operands_jp)/sizeof(operandlist), operands_jp},
    {"jr",       EZ80, 0, sizeof(operands_jr)/sizeof(operandlist), operands_jr},
    {"ld",       EZ80, 0, sizeof(operands_ld)/sizeof(operandlist), operands_ld},
    {"ldd",      EZ80, 0, sizeof(operands_ldd)/sizeof(operandlist), operands_ldd},
    {"lddr",     EZ80, 0, sizeof(operands_lddr)/sizeof(operandlist), operands_lddr},
    {"ldi",      EZ80, 0, sizeof(operands_ldi)/sizeof(operandlist), operands_ldi},
    {"ldir",     EZ80, 0, sizeof(operands_ldir)/sizeof(operandlist), operands_ldir},
    {"lea",      EZ80, 0, sizeof(operands_lea)/sizeof(operandlist), operands_lea},
    {"macro",    ASSEMBLER, ASM_MACRO_START, 0, NULL, ASM_ARG_LIST},
    {"mlt",      EZ80, 0, sizeof(operands_mlt)/sizeof(operandlist), operands_mlt},
    {"neg",      EZ80, 0, sizeof(operands_neg)/sizeof(operandlist), operands_neg},
    {"nop",      EZ80, 0, sizeof(operands_nop)/sizeof(operandlist), operands_nop},
    {"or",       EZ80, 0, sizeof(operands_or)/sizeof(operandlist), operands_or},
    {"org",     ASSEMBLER, ASM_ORG, 0, NULL, ASM_ARG_SINGLE},
    {"otd2r",    EZ80, 0, sizeof(operands_otd2r)/sizeof(operandlist), operands_otd2r},
    {"otdm",     EZ80, 0, sizeof(operands_otdm)/sizeof(operandlist), operands_otdm},
    {"otdmr",    EZ80, 0, sizeof(operands_otdmr)/sizeof(operandlist), operands_otdmr},
    {"otdr",     EZ80, 0, sizeof(operands_otdr)/sizeof(operandlist), operands_otdr},
    {"otdrx",    EZ80, 0, sizeof(operands_otdrx)/sizeof(operandlist), operands_otdrx},
    {"oti2r",    EZ80, 0, sizeof(operands_oti2r)/sizeof(operandlist), operands_oti2r},
    {"otim",     EZ80, 0, sizeof(operands_otim)/sizeof(operandlist), operands_otim},
    {"otimr",    EZ80, 0, sizeof(operands_otimr)/sizeof(operandlist), operands_otimr},
    {"otir",     EZ80, 0, sizeof(operands_otir)/sizeof(operandlist), operands_otir},
    {"otirx",    EZ80, 0, sizeof(operands_otirx)/sizeof(operandlist), operands_otirx},
    {"out",      EZ80, 0, sizeof(operands_out)/sizeof(operandlist), operands_out},
    {"out0",     EZ80, 0, sizeof(operands_out0)/sizeof(operandlist), operands_out0},
    {"outd",     EZ80, 0, sizeof(operands_outd)/sizeof(operandlist), operands_outd},
    {"outd2",    EZ80, 0, sizeof(operands_outd2)/sizeof(operandlist), operands_outd2},
    {"outi",     EZ80, 0, sizeof(operands_outi)/sizeof(operandlist), operands_outi},
    {"outi2",    EZ80, 0, sizeof(operands_outi2)/sizeof(operandlist), operands_outi2},
    {"pea",      EZ80, 0, sizeof(operands_pea)/sizeof(operandlist), operands_pea},
    {"pop",      EZ80, 0, sizeof(operands_pop)/sizeof(operandlist), operands_pop},
    {"push",     EZ80, 0, sizeof(operands_push)/sizeof(operandlist), operands_push},
    {"res",      EZ80, 0, sizeof(operands_res)/sizeof(operandlist), operands_res},
    {"ret",      EZ80, 0, sizeof(operands_ret)/sizeof(operandlist), operands_ret},
    {"reti",     EZ80, 0, sizeof(operands_reti)/sizeof(operandlist), operands_reti},
    {"retn",     EZ80, 0, sizeof(operands_retn)/sizeof(operandlist), operands_retn},
    {"rl",       EZ80, 0, sizeof(operands_rl)/sizeof(operandlist), operands_rl},
    {"rla",      EZ80, 0, sizeof(operands_rla)/sizeof(operandlist), operands_rla},
    {"rlc",      EZ80, 0, sizeof(operands_rlc)/sizeof(operandlist), operands_rlc},
    {"rlca",     EZ80, 0, sizeof(operands_rlca)/sizeof(operandlist), operands_rlca},
    {"rld",      EZ80, 0, sizeof(operands_rld)/sizeof(operandlist), operands_rld},
    {"rr",       EZ80, 0, sizeof(operands_rr)/sizeof(operandlist), operands_rr},
    {"rra",      EZ80, 0, sizeof(operands_rra)/sizeof(operandlist), operands_rra},
    {"rrc",      EZ80, 0, sizeof(operands_rrc)/sizeof(operandlist), operands_rrc},
    {"rrca",     EZ80, 0, sizeof(operands_rrca)/sizeof(operandlist), operands_rrca},
    {"rrd",      EZ80, 0, sizeof(operands_rrd)/sizeof(operandlist), operands_rrd},
    {"rsmix",    EZ80, 0, sizeof(operands_rsmix)/sizeof(operandlist), operands_rsmix},
    {"rst",      EZ80, 0, sizeof(operands_rst)/sizeof(operandlist), operands_rst},
    {"sbc",      EZ80, 0, sizeof(operands_sbc)/sizeof(operandlist), operands_sbc},
    {"scf",      EZ80, 0, sizeof(operands_scf)/sizeof(operandlist), operands_scf},
    {"set",      EZ80, 0, sizeof(operands_set)/sizeof(operandlist), operands_set},
    {"sla",      EZ80, 0, sizeof(operands_sla)/sizeof(operandlist), operands_sla},
    {"slp",      EZ80, 0, sizeof(operands_slp)/sizeof(operandlist), operands_slp},
    {"sra",      EZ80, 0, sizeof(operands_sra)/sizeof(operandlist), operands_sra},
    {"srl",      EZ80, 0, sizeof(operands_srl)/sizeof(operandlist), operands_srl},
    {"stmix",    EZ80, 0, sizeof(operands_stmix)/sizeof(operandlist), operands_stmix},
    {"sub",      EZ80, 0, sizeof(operands_sub)/sizeof(operandlist), operands_sub},
    {"tst",      EZ80, 0, sizeof(operands_tst)/sizeof(operandlist), operands_tst},
    {"tstio",    EZ80, 0, sizeof(operands_tstio)/sizeof(operandlist), operands_tstio},
    {"xor",      EZ80, 0, sizeof(operands_xor)/sizeof(operandlist), operands_xor}
};

// Binary search of instruction table
// Requires a pre-sorted table
instruction * instruction_table_lookup(char *key){
	instruction *base = instructions;
	int lim, cmp;
	instruction *p;

	for (lim = sizeof(instructions)/sizeof(instruction); lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcasecmp(key,p->name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (NULL);
}

