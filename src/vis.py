import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("task_log.csv")
task_logs = df[df['Type'] == 'TASK']

task_names = task_logs['Task Name'].unique()
task_map = {name: i for i, name in enumerate(task_names)}

fig, ax = plt.subplots(figsize=(10, 5))
for _, row in task_logs.iterrows():
    y = task_map[row['Task Name']]
    ax.broken_barh([(row['Tick'], 1)], (y - 0.4, 0.8), facecolors='tab:blue')

ax.set_yticks(range(len(task_names)))
ax.set_yticklabels(task_names)
ax.set_xlabel('Tick')
ax.set_title('RTOS Task Execution Timeline')
ax.grid(True)

plt.tight_layout()
plt.savefig("rtos_timeline_gantt.png")
plt.show()
