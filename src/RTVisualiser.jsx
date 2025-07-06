// RTOSVisualizer.jsx — Enhanced React + D3 Gantt Viewer with Controls

import React, { useEffect, useRef, useState } from 'react';
import * as d3 from 'd3';
export default function RTVisualiser() {
  const ref = useRef();
  const [data, setData] = useState([]);
  const [tick, setTick] = useState(0);
  const [playing, setPlaying] = useState(true);
  const [speed, setSpeed] = useState(500); // ms per tick
  const [filterState, setFilterState] = useState('ALL');

  const colorMap = {
    RUNNING: '#4caf50',
    READY: '#2196f3',
    DELAYED: '#ffc107',
    WAITING_EVENT: '#9c27b0',
    INTERRUPTED: '#f44336',
    SLEEP: '#90a4ae',
    BLOCKED: '#795548',
    TERMINATED: '#bdbdbd'
  };

  useEffect(() => {
    d3.csv('/task_log.csv').then(csv => {
      csv.forEach(d => {
        d.Tick = +d.Tick;
        d.TaskID = +d['Task ID'];
        d.State = d.State.toUpperCase();
      });
      setData(csv);
    });
  }, []);

  useEffect(() => {
    if (!playing) return;
    const interval = setInterval(() => setTick(t => t + 1), speed);
    return () => clearInterval(interval);
  }, [playing, speed]);

  useEffect(() => {
    if (data.length === 0) return;
    const svg = d3.select(ref.current);
    svg.selectAll('*').remove();

    const margin = { top: 40, right: 30, bottom: 40, left: 120 };
    const width = 960 - margin.left - margin.right;
    const height = 500 - margin.top - margin.bottom;

    const taskNames = Array.from(new Set(data.map(d => d['Task Name'])));
    const x = d3.scaleLinear().domain([0, tick + 5]).range([0, width]);
    const y = d3.scaleBand().domain(taskNames).range([0, height]).padding(0.2);

    const chart = svg.append('g').attr('transform', `translate(${margin.left},${margin.top})`);

    chart.append('g').call(d3.axisLeft(y));
    chart.append('g').attr('transform', `translate(0,${height})`).call(d3.axisBottom(x).ticks(10));

    const filtered = data.filter(d =>
      d.Tick <= tick && (filterState === 'ALL' || d.State === filterState)
    );

    chart.selectAll('rect')
      .data(filtered)
      .enter()
      .append('rect')
      .attr('x', d => x(d.Tick))
      .attr('y', d => y(d['Task Name']))
      .attr('width', 10)
      .attr('height', y.bandwidth())
      .attr('fill', d => colorMap[d.State] || '#ccc')
      .append('title')
      .text(d => `${d['Task Name']} | Tick ${d.Tick} | ${d.State}`);

    svg.selectAll('text.title').data([0]).join('text')
      .attr('x', margin.left + width / 2)
      .attr('y', 25)
      .attr('text-anchor', 'middle')
      .attr('class', 'title')
      .attr('font-size', '18px')
      .text(`RTOS Gantt Timeline (Tick: ${tick})`);

  }, [data, tick, filterState]);

  return (
    <div style={{ padding: '20px' }}>
      <h2>RTOS Visualizer with Controls</h2>
      <div style={{ marginBottom: '10px' }}>
        <button onClick={() => setPlaying(!playing)}>{playing ? 'Pause' : 'Play'}</button>
        <label style={{ marginLeft: '20px' }}>
          Speed:
          <input
            type="range"
            min="100"
            max="2000"
            step="100"
            value={speed}
            onChange={e => setSpeed(+e.target.value)}
          /> {speed}ms
        </label>
        <label style={{ marginLeft: '20px' }}>
          Filter State:
          <select onChange={e => setFilterState(e.target.value)} value={filterState}>
            <option value="ALL">All</option>
            {Object.keys(colorMap).map(s => (
              <option key={s} value={s}>{s}</option>
            ))}
          </select>
        </label>
      </div>
      <svg ref={ref} width={960} height={500} style={{ border: '1px solid #ccc' }}></svg>
    </div>
  );
}
