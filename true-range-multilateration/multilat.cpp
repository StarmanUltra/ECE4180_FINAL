typedef struct PointLatLon
{
    float lat;
    int lon;
};

typedef void (*functiontype)();

class Multilat
{
public:
    int number_of_stations;
    PointLatLon station_locations[number_of_stations];

    PointLatLon x0 = {0, 0};
    float tol = 1E-6;
    int max_itter = 1000;




    PointLatLon locate_strike(void);
};
