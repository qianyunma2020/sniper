#!/usr/bin/env python

def HelloJob():
    # configuration is just the same as a single thread job
    import Sniper
    global ith
    ith += 1
    task = Sniper.Task(str(ith) +"-Job")
    task.setLogLevel(3)

    import HelloWorld
    task.property("algs").append("HelloAlg/x")

    x = task.find("x")
    x.property("VarString").set("GOD")
    x.property("VectorInt").set(range(6))
    x.property("MapStrInt").set( {"str%d"%v:v for v in range(6)} )

    global first_time
    if first_time:
        task.show()
        first_time = False
    # instead of task.run(), we must return the task object here
    return task


if __name__ == "__main__":

    import AlgLevelParallel
