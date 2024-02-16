#include "../band.h"
#include "../error.h"
#include "../codegen.h"

extern region_t* to_str(FILE* out, band_t* band, const char* _arg) {
    region_t* arg = band_region_for_var(band, _arg);
    if (!arg) {
        panic("argument has to be a variable");
    }

    region_t* str = band_allocate_tmp(band, 3);
    for (size_t i = 0; i < str->size; i++) {
        move_to(str->start + i); reset();
    }

    region_t* copy = band_allocate_tmp(band, 1);
    move_to(copy->start); reset();

    region_t* ten = band_allocate_tmp(band, 1);
    move_to(ten->start); reset(); add(10);

    region_t* tmp = band_allocate_tmp(band, 1);
    move_to(tmp->start); reset();
    region_t* tmp2 = band_allocate_tmp(band, 1);
    move_to(tmp2->start); reset();
    region_t* tmp3 = band_allocate_tmp(band, 1);

    move_to(arg->start);
    loop({
        dec();
        move_to(tmp->start); inc();
        move_to(copy->start); inc();
        move_to(str->start + 2); inc();
        move_to(str->start + 1); reset();
        move_to(str->start + 0); reset();
        move_to(ten->start); dec();
        loop({
            dec();
            move_to(str->start + 0); inc();
            move_to(str->start + 1); inc();
            move_to(ten->start);
        });
        move_to(str->start + 1);
        loop({
            move_to(tmp->start); reset();
            move_to(str->start + 1); reset();
        });
        move_to(str->start + 0);
        loop({
            dec();
            move_to(ten->start); inc();
            move_to(str->start + 0);
        });
        move_to(tmp->start);
        loop({
            move_to(tmp2->start); inc();
            move_to(str->start + 2); reset();
            move_to(ten->start); add(10);
            move_to(tmp->start); reset();
        });

        move_to(arg->start);
    });

    move_to(copy->start);
    loop({
        dec();
        move_to(arg->start); inc();
        move_to(copy->start);
    });

    move_to(ten->start); reset(); add(10);

    move_to(tmp2->start);
    loop({
        dec();
        move_to(tmp->start); reset(); inc();
        move_to(str->start + 1); inc();
        move_to(copy->start); reset();
        move_to(tmp3->start); reset();
        move_to(ten->start); dec();
        loop({
            dec();
            move_to(copy->start); inc();
            move_to(tmp3->start); inc();
            move_to(ten->start);
        });
        move_to(tmp3->start);
        loop({
            move_to(tmp->start); reset();
            move_to(tmp3->start); reset();
        });
        move_to(copy->start);
        loop({
            dec();
            move_to(ten->start); inc();
            move_to(copy->start);
        });

        move_to(tmp->start);
        loop({
            move_to(ten->start); add(10);
            move_to(str->start + 0); inc();
            move_to(str->start + 1); reset();
            move_to(tmp->start); reset();
        });

        move_to(tmp2->start);
    });

    for (size_t i = 0; i < str->size; i++) {
        move_to(str->start + i); add('0');
    }

    band_region_free(band, tmp3);
    band_region_free(band, tmp2);
    band_region_free(band, tmp);
    band_region_free(band, ten);

    return str;
}
