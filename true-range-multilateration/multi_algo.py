
import numpy as np
# from numpy.lib.function_base import meshgrid
from scipy.optimize import minimize
import math
from collections import namedtuple
from functools import partial
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.patches import Ellipse





LatLonPoint = namedtuple('LatLonPoint', ['lat', 'lon'])


def d_haversine(point_a, point_b):
	pi_on_180 = 0.017453292519943295

	lat1 = point_a.lat*pi_on_180
	lat2 = point_b.lat*pi_on_180

	lon1 = point_a.lon*pi_on_180
	lon2 = point_b.lon*pi_on_180

	dlon = lon2 - lon1 
	dlat = lat2 - lat1 
	a = math.sin(dlat/2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon/2)**2
	c = 2 * math.asin(math.sqrt(a)) 

	d = 6371* c
	return d

def d_equirectangular(point_a, point_b):
	pi_on_180 = 0.017453292519943295

	lat1 = point_a.lat*pi_on_180
	lat2 = point_b.lat*pi_on_180

	lon1 = point_a.lon*pi_on_180
	lon2 = point_b.lon*pi_on_180

	x = (lon2 - lon1) * math.cos( 0.5*(lat2 + lat1) )
	y = lat2 - lat1
	d = 6371 * math.sqrt( x*x + y*y )
	return d



range_points=[40,37,34,31,27,24,20,17,14,12,10,8,6,5,0]
def round_to_range_points(range):
    deltas = [abs(range-p) for p in range_points]
    rounded_value = range_points[deltas.index(min(deltas))]
    return rounded_value


def objective_function(x, station_locations=[], station_ranges=[]):
    strike_location_guess = LatLonPoint(x[0], x[1])
    error = 0;
    for station_location, station_range in zip(station_locations, station_ranges):
        error += abs(d_haversine(station_location, strike_location_guess) - station_range)
    return error
    

def locate_strike(station_locations, station_ranges, verbose=False):
    x0 = [-33.0, -80.0]
    if verbose:
        print(station_ranges)
    
    obj_func = partial(objective_function,station_locations=station_locations,station_ranges=station_ranges)
    res = minimize(obj_func, x0, method='nelder-mead',options={'maxiter':5000, 'maxfev':5000, 'fatol':1e-6})
    if verbose:
        print(res)

    return LatLonPoint(res.x[0], res.x[1])


if __name__ == "__main__":
    station_locations = [LatLonPoint(lat=33.778662, lon=-84.408694), # 935 apartment
                         LatLonPoint(lat=33.769620, lon=-84.390898), # nav east apartment
                         LatLonPoint(lat=33.781994, lon=-84.402854)] # center street house
    strike_location = LatLonPoint(33.641154, -84.435819) # Bobby Jones Golf Course
    print(f"Strike Location Exact: {strike_location}")

    station_ranges_haversine = [d_haversine(station, strike_location) for station in station_locations]
    station_ranges_equirectangular = [d_equirectangular(station, strike_location) for station in station_locations]

    print(station_ranges_haversine)
    print(f"Station Ranges Excat: {station_ranges_haversine}")

    station_ranges_rounded = [round_to_range_points(x) for x in station_ranges_haversine]
    # print(f"Station Ranges Rounded: {station_ranges_rounded}")

    ans = locate_strike(station_locations, station_ranges_rounded)
    print(f"Predicted Strike Location: {ans}")
    
    error = d_haversine(ans, strike_location)
    print(f"Predicted Strike Location Error: {error}")
    

    obj_func = partial(objective_function,station_locations=station_locations,station_ranges=station_ranges_haversine)
    lats = np.arange(33.5, 34, 0.005)
    lons = np.arange(-84.6, -84.2, 0.005)

    lons_v, lats_v = np.meshgrid(lons, lats)

    coords = [ [c[0],c[1]] for c in zip(lats_v.ravel().tolist(), lons_v.ravel().tolist())] 

    z = list(map(obj_func, coords))
    z = np.reshape(z, (len(lats), len(lons)))
    
    # fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
    # surf = ax.plot_surface(lons_v, lats_v, z, linewidth=10, antialiased=True, cmap='coolwarm', alpha=0.9)
    # ax.set_xlabel('Longitude')
    # ax.set_ylabel('Latitude')
    # ax.set_zlabel('Objective Function (Error)')



    fig, ax = plt.subplots()
    # ax.scatter(lons_v.ravel().tolist(), lats_v.ravel().tolist())
    pcolormesh = ax.pcolormesh(lons_v, lats_v, z, vmin=0)

    ellipses = [   Ellipse((i[0].lon, i[0].lat), width=i[1]/88*2, height=i[1]/111*2, color='y', fill=False) for i in zip(station_locations, station_ranges_rounded) ] 
    for e in ellipses:
        ax.add_patch(e)
        print(e)
    
    ax.scatter(strike_location.lon, strike_location.lat, s=100, c='g', marker='x')
    ax.scatter(ans.lon, ans.lat, s=100, c='r', marker='+')

    ax.scatter([s.lon for s in station_locations], [s.lat for s in station_locations], s=50, c='b', marker='o')
    ax.set_xlabel('Longitude')
    ax.set_ylabel('Latitude')
    cbar = plt.colorbar(pcolormesh, ax=ax)
    cbar.set_label('Objective Function (Error)')

    plt.show()