#ifndef __LINUX_PWM_H
#define __LINUX_PWM_H

#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/of.h>

struct pwm_capture;
struct pwm_device;
struct seq_file;

#if IS_ENABLED(CONFIG_PWM)
/*
 * pwm_request - request a PWM device
 */
struct pwm_device *pwm_request(int pwm_id, const char *label);

/*
 * pwm_free - free a PWM device
 */
void pwm_free(struct pwm_device *pwm);

/*
 * pwm_config - change a PWM device configuration
 */
int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns);

/*
 * pwm_enable - start a PWM output toggling
 */
int pwm_enable(struct pwm_device *pwm);

/*
 * pwm_disable - stop a PWM output toggling
 */
void pwm_disable(struct pwm_device *pwm);

/*
 * Set PWM signal ramp up and down time.
 */
int pwm_set_ramp_time(struct pwm_device *pwm, int ramp_time);

/*
 * Set double pulse period.
 */
int pwm_set_double_pulse_period(struct pwm_device *pwm, int period);

/*
 * Set PWM capture window length.
 */
int pwm_set_capture_window_length(struct pwm_device *pwm, int window_length);

#else
static inline struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	return ERR_PTR(-ENODEV);
}

static inline void pwm_free(struct pwm_device *pwm)
{
}

static inline int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns)
{
	return -EINVAL;
}

static inline int pwm_enable(struct pwm_device *pwm)
{
	return -EINVAL;
}

static inline void pwm_disable(struct pwm_device *pwm)
{
}

static inline int pwm_set_ramp_time(struct pwm_device *pwm, int ramp_time)
{
	return -EINVAL;
}

static inline int pwm_set_double_pulse_period(struct pwm_device *pwm,
					      int period)
{
	return -EINVAL;
}

static inline int pwm_set_capture_window_length(struct pwm_device *pwm,
					       int window_length)
{
	return -EINVAL;
}
#endif

struct pwm_chip;

/**
 * enum pwm_polarity - polarity of a PWM signal
 * @PWM_POLARITY_NORMAL: a high signal for the duration of the duty-
 * cycle, followed by a low signal for the remainder of the pulse
 * period
 * @PWM_POLARITY_INVERSED: a low signal for the duration of the duty-
 * cycle, followed by a high signal for the remainder of the pulse
 * period
 */
enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};

enum {
	PWMF_REQUESTED = 1 << 0,
	PWMF_ENABLED = 1 << 1,
	PWMF_EXPORTED = 1 << 2,
};

/**
 * struct pwm_device - PWM channel object
 * @label: name of the PWM device
 * @flags: flags associated with the PWM device
 * @hwpwm: per-chip relative index of the PWM device
 * @pwm: global index of the PWM device
 * @chip: PWM chip providing this PWM device
 * @chip_data: chip-private data associated with the PWM device
 * @lock: used to serialize accesses to the PWM device where necessary
 * @period: period of the PWM signal (in nanoseconds)
 * @duty_cycle: duty cycle of the PWM signal (in nanoseconds)
 * @double_period: Doble pulse period.
 * @ramp_time: Ramp up/down time.
 * @capture_win_len: Window length for captureing PWM signal.
 * @polarity: polarity of the PWM signal
 */
struct pwm_device {
	const char *label;
	unsigned long flags;
	unsigned int hwpwm;
	unsigned int pwm;
	struct pwm_chip *chip;
	void *chip_data;
	struct mutex lock;

	unsigned int period;
	unsigned int duty_cycle;
	enum pwm_polarity polarity;
	unsigned int double_period;
	unsigned int ramp_time;
	unsigned int capture_win_len;;
};

static inline bool pwm_is_enabled(const struct pwm_device *pwm)
{
	return test_bit(PWMF_ENABLED, &pwm->flags);
}

static inline void pwm_set_period(struct pwm_device *pwm, unsigned int period)
{
	if (pwm)
		pwm->period = period;
}

static inline unsigned int pwm_get_period(const struct pwm_device *pwm)
{
	return pwm ? pwm->period : 0;
}

static inline void pwm_set_duty_cycle(struct pwm_device *pwm, unsigned int duty)
{
	if (pwm)
		pwm->duty_cycle = duty;
}

static inline unsigned int pwm_get_duty_cycle(const struct pwm_device *pwm)
{
	return pwm ? pwm->duty_cycle : 0;
}

static inline unsigned int pwm_get_double_period(const struct pwm_device *pwm)
{
	return pwm ? pwm->double_period : 0;
}

static inline unsigned int pwm_get_ramp_time(const struct pwm_device *pwm)
{
	return pwm ? pwm->ramp_time : 0;
}

static inline unsigned int pwm_get_capture_window_length(
					const struct pwm_device *pwm)
{
	return pwm ? pwm->capture_win_len : 0;
}

/*
 * pwm_set_polarity - configure the polarity of a PWM signal
 */
int pwm_set_polarity(struct pwm_device *pwm, enum pwm_polarity polarity);

static inline enum pwm_polarity pwm_get_polarity(const struct pwm_device *pwm)
{
	return pwm ? pwm->polarity : PWM_POLARITY_NORMAL;
}

/**
 * struct pwm_ops - PWM controller operations
 * @request: optional hook for requesting a PWM
 * @free: optional hook for freeing a PWM
 * @config: configure duty cycles and period length for this PWM
 * @set_polarity: configure the polarity of this PWM
 * @capture: capture and report PWM signal
 * @enable: enable PWM output toggling
 * @disable: disable PWM output toggling
 * @set_ramp_time: Set PWM ramp up/down time.
 * @set_double_pulse_period: Set double pulse period time.
 * @set_capture_window_length: Set PWM capture window length.
 * @dbg_show: optional routine to show contents in debugfs
 * @owner: helps prevent removal of modules exporting active PWMs
 */
struct pwm_ops {
	int (*request)(struct pwm_chip *chip, struct pwm_device *pwm);
	void (*free)(struct pwm_chip *chip, struct pwm_device *pwm);
	int (*config)(struct pwm_chip *chip, struct pwm_device *pwm,
		      int duty_ns, int period_ns);
	int (*set_polarity)(struct pwm_chip *chip, struct pwm_device *pwm,
			    enum pwm_polarity polarity);
	int (*capture)(struct pwm_chip *chip, struct pwm_device *pwm,
		       struct pwm_capture *result, unsigned long timeout);
	int (*enable)(struct pwm_chip *chip, struct pwm_device *pwm);
	void (*disable)(struct pwm_chip *chip, struct pwm_device *pwm);
	int (*set_ramp_time)(struct pwm_chip *chip, struct pwm_device *pwm,
			     int ramp_time);
	int (*set_double_pulse_period)(struct pwm_chip *chip,
				       struct pwm_device *pwm,
				       int period);
	int (*set_capture_window_length)(struct pwm_chip *chip,
					 struct pwm_device *pwm,
					 int window_length);
#ifdef CONFIG_DEBUG_FS
	void (*dbg_show)(struct pwm_chip *chip, struct seq_file *s);
#endif
	struct module *owner;
};

/**
 * struct pwm_chip - abstract a PWM controller
 * @dev: device providing the PWMs
 * @list: list node for internal use
 * @ops: callbacks for this PWM controller
 * @base: number of first PWM controlled by this chip
 * @npwm: number of PWMs controlled by this chip
 * @pwms: array of PWM devices allocated by the framework
 * @of_xlate: request a PWM device given a device tree PWM specifier
 * @of_pwm_n_cells: number of cells expected in the device tree PWM specifier
 * @can_sleep: must be true if the .config(), .enable() or .disable()
 *             operations may sleep
 */
struct pwm_chip {
	struct device *dev;
	struct list_head list;
	const struct pwm_ops *ops;
	int base;
	unsigned int npwm;

	struct pwm_device *pwms;

	struct pwm_device * (*of_xlate)(struct pwm_chip *pc,
					const struct of_phandle_args *args);
	unsigned int of_pwm_n_cells;
	bool can_sleep;
};

/**
 * struct pwm_capture - PWM capture data
 * @period: period of the PWM signal (in nanoseconds)
 * @duty_cycle: duty cycle of the PWM signal (in nanoseconds)
 */
struct pwm_capture {
	unsigned int period;
	unsigned int duty_cycle;
};

#if IS_ENABLED(CONFIG_PWM)
int pwm_capture(struct pwm_device *pwm, struct pwm_capture *result,
		unsigned long timeout);
int pwm_set_chip_data(struct pwm_device *pwm, void *data);
void *pwm_get_chip_data(struct pwm_device *pwm);

int pwmchip_add_with_polarity(struct pwm_chip *chip,
			      enum pwm_polarity polarity);
int pwmchip_add(struct pwm_chip *chip);
int pwmchip_remove(struct pwm_chip *chip);
struct pwm_device *pwm_request_from_chip(struct pwm_chip *chip,
					 unsigned int index,
					 const char *label);

struct pwm_device *of_pwm_xlate_with_flags(struct pwm_chip *pc,
		const struct of_phandle_args *args);

struct pwm_device *pwm_get(struct device *dev, const char *con_id);
struct pwm_device *of_pwm_get(struct device_node *np, const char *con_id);
void pwm_put(struct pwm_device *pwm);

struct pwm_device *devm_pwm_get(struct device *dev, const char *con_id);
struct pwm_device *devm_of_pwm_get(struct device *dev, struct device_node *np,
				   const char *con_id);
void devm_pwm_put(struct device *dev, struct pwm_device *pwm);

bool pwm_can_sleep(struct pwm_device *pwm);
#else
static inline int pwm_capture(struct pwm_device *pwm,
			      struct pwm_capture *result,
			      unsigned long timeout)
{
	return -EINVAL;
}

static inline int pwm_set_chip_data(struct pwm_device *pwm, void *data)
{
	return -EINVAL;
}

static inline void *pwm_get_chip_data(struct pwm_device *pwm)
{
	return NULL;
}

static inline int pwmchip_add(struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline int pwmchip_add_inversed(struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline int pwmchip_remove(struct pwm_chip *chip)
{
	return -EINVAL;
}

static inline struct pwm_device *pwm_request_from_chip(struct pwm_chip *chip,
						       unsigned int index,
						       const char *label)
{
	return ERR_PTR(-ENODEV);
}

static inline struct pwm_device *pwm_get(struct device *dev,
					 const char *consumer)
{
	return ERR_PTR(-ENODEV);
}

static inline struct pwm_device *of_pwm_get(struct device_node *np,
					    const char *con_id)
{
	return ERR_PTR(-ENODEV);
}

static inline void pwm_put(struct pwm_device *pwm)
{
}

static inline struct pwm_device *devm_pwm_get(struct device *dev,
					      const char *consumer)
{
	return ERR_PTR(-ENODEV);
}

static inline struct pwm_device *devm_of_pwm_get(struct device *dev,
						 struct device_node *np,
						 const char *con_id)
{
	return ERR_PTR(-ENODEV);
}

static inline void devm_pwm_put(struct device *dev, struct pwm_device *pwm)
{
}

static inline bool pwm_can_sleep(struct pwm_device *pwm)
{
	return false;
}
#endif

struct pwm_lookup {
	struct list_head list;
	const char *provider;
	unsigned int index;
	const char *dev_id;
	const char *con_id;
	unsigned int period;
	enum pwm_polarity polarity;
};

#define PWM_LOOKUP(_provider, _index, _dev_id, _con_id, _period, _polarity) \
	{						\
		.provider = _provider,			\
		.index = _index,			\
		.dev_id = _dev_id,			\
		.con_id = _con_id,			\
		.period = _period,			\
		.polarity = _polarity			\
	}

#if IS_ENABLED(CONFIG_PWM)
void pwm_add_table(struct pwm_lookup *table, size_t num);
void pwm_remove_table(struct pwm_lookup *table, size_t num);
#else
static inline void pwm_add_table(struct pwm_lookup *table, size_t num)
{
}

static inline void pwm_remove_table(struct pwm_lookup *table, size_t num)
{
}
#endif

#ifdef CONFIG_PWM_SYSFS
void pwmchip_sysfs_export(struct pwm_chip *chip);
void pwmchip_sysfs_unexport(struct pwm_chip *chip);
void pwmchip_sysfs_unexport_children(struct pwm_chip *chip);
#else
static inline void pwmchip_sysfs_export(struct pwm_chip *chip)
{
}

static inline void pwmchip_sysfs_unexport(struct pwm_chip *chip)
{
}

static inline void pwmchip_sysfs_unexport_children(struct pwm_chip *chip)
{
}
#endif /* CONFIG_PWM_SYSFS */

#endif /* __LINUX_PWM_H */
