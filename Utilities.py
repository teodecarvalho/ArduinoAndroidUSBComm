from time import sleep

def try_n_times(func, max_tries = 10, sleep_time = 1, **kwargs):
    for i in range(0, max_tries):
        try:
            return func(**kwargs)
        except:
            sleep(sleep_time)
    raise Exception("Something went wrong with the function '" + func.__name__ + "'!")