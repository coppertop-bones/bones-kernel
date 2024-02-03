


def fred():
    pass


    # C source

    # int main() {
    #     int a;
    #     a = 1;
    #     a += 1;
    #     return a;
    # }

    # bones source
    # main: {:i32
    #     a: 1
    #     a: a + 1
    #     ^ a
    # }

    # target QBE
    #
    # export function w $main() {
    # @start.1
    # @body.2
    #     %_a =l alloc4 4
    #     storew 1, %_a
    #     %.5 =w loadw %_a
    #     %.4 =w add %.5, 1
    #     storew %.4, %_a
    #     %.7 =w loadw %_a
    #     ret %.7
    # }

