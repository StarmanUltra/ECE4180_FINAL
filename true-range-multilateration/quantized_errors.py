from multi_algo import LatLonPoint, d_haversine, round_to_range_points, locate_strike
import numpy as np

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.patches import Ellipse

from joblib import Parallel, delayed
from tqdm import tqdm


station_locations = [LatLonPoint(lat=33.778662, lon=-84.408694), # 935 apartment
                        LatLonPoint(lat=33.769620, lon=-84.390898), # nav east apartment
                        LatLonPoint(lat=33.781994, lon=-84.402854)] # center street house

lats = np.arange(33.5, 34.2, 0.01)
lons = np.arange(-84.8, -84.0, 0.01)
lons_v, lats_v = np.meshgrid(lons, lats)

strike_locations = [ LatLonPoint(c[0],c[1]) for c in zip(lats_v.ravel().tolist(), lons_v.ravel().tolist())]


def calculate_error_single_station(strike_location):
    station_ranges_haversine = [d_haversine(station, strike_location) for station in station_locations]
    station_ranges_rounded = [round_to_range_points(x) for x in station_ranges_haversine]
    ans = locate_strike(station_locations, station_ranges_rounded)
    error = d_haversine(ans, strike_location)
    return error

z = Parallel(n_jobs=-1)(delayed(calculate_error_single_station)(i) for i in tqdm(strike_locations))


z = np.array(z)
z = np.reshape(z, (len(lats), len(lons)))

fig, ax = plt.subplots()
pcolormesh = ax.pcolormesh(lons_v, lats_v, z)

ranges = [1,2,5,10,15,20,30]

ellipses = [  Ellipse((station_locations[0].lon, station_locations[0].lat), width=(i/88*2), height=(i/111*2), fill=False, edgecolor=np.random.rand(3,), linewidth=2) for i in ranges ] 
for e in ellipses:
    ax.add_patch(e)
    print(e)
ax.legend(ellipses, ['Range: {} Km.'.format(i) for i in ranges])




ax.set_xlabel('Longitude')
ax.set_ylabel('Latitude')
cbar = plt.colorbar(pcolormesh, ax=ax)
cbar.set_label('Prediction Error (Km)')

ax.set_title("Station Range Quantization Error")
plt.show()
