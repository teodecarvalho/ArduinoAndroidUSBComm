from functools import wraps

def run_safely(func, max_tries = 10, sleep_time = .5):
    from time import sleep
    @wraps(func)
    def wrapper(*args, **kwargs):
        for i in range(0, max_tries):
            try:
                return func(*args, **kwargs)
            except:
                sleep(sleep_time)
        raise Exception("Something went wrong with the function '" + func.__name__ + "'!")
    return wrapper

def debug(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        print(f"***Function {func.__name__} was called with the following arguments: {args}, and {kwargs}.")
        result = func(*args, **kwargs)
        print(f"***Function {func.__name__} returned: {result}")
        return result
    return wrapper